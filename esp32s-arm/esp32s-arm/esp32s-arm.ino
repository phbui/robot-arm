#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>
#include <queue>  // Include queue library for command queue

// Wi-Fi credentials
const char *ssid = "Phi";
const char *password = "lukaspark";

// WebSocket server details
const char *websocket_host = "172.20.10.11";
const int websocket_port = 8080;
const char *websocket_path = "/";

// Motor driver control pins for stepper motor
const int STEP_PIN = 26; 
const int DIR_PIN = 25; 
const int SERVO_PIN = 19;
const int PEN_PIN = 18;   

// LED pin (Blue light to indicate WebSocket connection)
const int LED_PIN = 2; 

// Servo control for theta2 and pen
Servo servoTheta2;
Servo penServo;

// Variables for current positions
float currentTheta1 = 0.0;
float currentTheta2 = 0.0;
float targetServoTheta = 0.0;

// Movement status flags
bool movingStepper = false;
bool movingServo = false;

// Define a struct to hold movement commands
struct MovementCommand {
  float theta1;
  float theta2;
  bool penState;
};

// Create a queue to hold movement commands
std::queue<MovementCommand> commandQueue;

WebSocketsClient webSocket;

// Variables for non-blocking stepper movement
int currentStep = 0;
int totalSteps = 0;

// Function to log messages with timestamps
void logMessage(const char* level, const char* message) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] ");
  Serial.print(level);
  Serial.print(": ");
  Serial.println(message);
}

// Function to start stepper movement
void startMoveStepper(float targetTheta1) {
  String message = "[Stepper] Starting move to theta1: " + String(targetTheta1) + " from currentTheta1: " + String(currentTheta1);
  logMessage("INFO", message.c_str());

  digitalWrite(DIR_PIN, targetTheta1 > currentTheta1 ? HIGH : LOW);
  totalSteps = abs(targetTheta1 - currentTheta1) * 10;
  currentStep = 0;
  movingStepper = totalSteps > 0 ;

  message = "[Stepper] Total Steps to Move: " + String(totalSteps);
  logMessage("INFO", message.c_str());
}

// Function to handle stepper movement updates
void updateStepper() {
  String message;

  if (movingStepper && currentStep < totalSteps) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); // Adjust delay for speed
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
    currentStep++;

    logMessage("DEBUG", "Updating stepper movement...");
    message = "Current Step: " + String(currentStep);
    logMessage("DEBUG", message.c_str());

    if (currentStep >= totalSteps) {
      movingStepper = false;
      currentTheta1 = (totalSteps / 10.0) * (digitalRead(DIR_PIN) == HIGH ? 1 : -1) + currentTheta1;
        
      message = "[Stepper] Movement Completed, newTheta1: " + String(currentTheta1);
      logMessage("INFO", message.c_str());
      // Serial.println(String(movingStepper).c_str());
      // Serial.println(String(movingServo).c_str());
      // Serial.println(String(currentStep).c_str());
      // Serial.println(String(totalSteps).c_str());
    }
  }
}

// Function to start servo movement
void startMoveServo(float targetTheta2) {
  String message = "[Servo] Starting move to theta2: " + String(targetTheta2) + " from currentTheta2: " + String(currentTheta2);
  logMessage("INFO", message.c_str());

  targetServoTheta = targetTheta2;
  movingServo = true;
}

// Function to handle servo movement updates
void updateServo() {
  String message;

  if (movingServo && currentTheta2 != targetServoTheta) {
    currentTheta2 += (currentTheta2 < targetServoTheta) ? 0.5 : -0.5;
    servoTheta2.write(currentTheta2);
    delay(20);

    // logMessage("DEBUG", "Updating servo movement...");
    // message = "Current Theta2: " + String(currentTheta2);
    // logMessage("DEBUG", message.c_str());

    if (abs(currentTheta2 - targetServoTheta) < 0.5) {
      currentTheta2 = targetServoTheta;
      movingServo = false;

      message = "[Servo] Movement Completed, newTheta2: " + String(currentTheta2);
      logMessage("INFO", message.c_str());
      // Serial.println(String(movingStepper).c_str());
      // Serial.println(String(movingServo).c_str());
      // Serial.println(String(currentTheta2).c_str());
      // Serial.println(String(targetServoTheta).c_str());
    }
  }
}

// Function to move the pen
void movePen(bool penState) {
  penServo.write(penState ? 180 : 0);
  logMessage("INFO", penState ? "Pen moved down." : "Pen moved up.");
}

// Function to start both stepper and servo movements
void startMoveTo(float targetTheta1, float targetTheta2, bool penState) {
  String message = "[Movement] Starting move to theta1: " + String(targetTheta1) + ", theta2: " + String(targetTheta2) + ", penState: " + (penState ? "true" : "false");
  logMessage("INFO", message.c_str());

  movePen(penState);
  startMoveStepper(targetTheta1);
  startMoveServo(targetTheta2);
}

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      logMessage("ERROR", "Disconnected from server.");
      digitalWrite(LED_PIN, LOW);
      break;
    case WStype_CONNECTED:
      logMessage("INFO", "Connected to server.");
      webSocket.sendTXT("{\"id\": \"ARM\"}");
      digitalWrite(LED_PIN, HIGH);
      break;
    case WStype_TEXT: {
      String message = "[WebSocket] Received message: " + String((char*)payload);
      logMessage("INFO", message.c_str());
      handleReceivedMessage(payload, length);
      break;
    }
    case WStype_ERROR:
      logMessage("ERROR", "WebSocket Error occurred.");
      break;
    default:
      break;
  }
}

// Function to handle received messages via WebSocket
void handleReceivedMessage(uint8_t *payload, size_t length) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    logMessage("ERROR", "Failed to deserialize JSON.");
    return;
  }

  const char *id = doc["id"];
  if (strcmp(id, "ARM") == 0) {
    JsonObject data = doc["data"];
    float theta1 = data["theta1"];
    float theta2 = data["theta2"];
    bool pen = data["pen"];
    commandQueue.push({ theta1, theta2, pen });

    String message = "[Queue] Queued movement - Theta1: " + String(theta1) + ", Theta2: " + String(theta2) + ", Pen: " + (pen ? "true" : "false");
    logMessage("INFO", message.c_str());
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  servoTheta2.attach(SERVO_PIN);
  penServo.attach(PEN_PIN);

  logMessage("INFO", "Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logMessage("DEBUG", "Waiting for WiFi connection...");
  }
  logMessage("INFO", "WiFi Connected.");

  logMessage("INFO", "Initializing WebSocket connection...");
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();

  // Serial.println("Stepper Moving: ");
  // Serial.println(String(movingStepper).c_str());
  // Serial.println("Servo Moving: ");
  // Serial.println(String(movingServo).c_str());

  // Check if the motors are not moving
  if (!movingStepper && !movingServo) {
    // If there are commands in the queue, process the next one
    if (!commandQueue.empty()) {
      MovementCommand cmd = commandQueue.front();
      commandQueue.pop();
      logMessage("INFO", "Processing next queued movement...");
      startMoveTo(cmd.theta1, cmd.theta2, cmd.penState);
    } else {
      //logMessage("INFO", "No more commands, nothing to do.");
    }
  }

  // Continuously update the stepper and servo movements
  if (movingStepper) {
    updateStepper();
  }
  
  if (movingServo) {
    updateServo();
  }
}
