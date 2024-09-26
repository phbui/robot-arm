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
const int STEP_PIN = 26; // BLUE
const int DIR_PIN = 25; // RED
const int SERVO_PIN = 19; // Servo control
const int PEN_PIN = 18;   // Pen servo

// LED pin (Blue light to indicate WebSocket connection)
const int LED_PIN = 2; // Onboard LED for many ESP32 boards, use another pin if using an external LED

// Servo control for theta2 and pen
Servo servoTheta2;
Servo penServo;

// Define home positions
const float HOME_THETA1 = 0.0;  // Home position for stepper motor
const float HOME_THETA2 = 0.0;  // Home position for servo

// Variables for current positions
float currentTheta1 = HOME_THETA1;
float currentTheta2 = HOME_THETA2;
float targetServoTheta = HOME_THETA2;

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

// Function to return both the stepper and servo to their home positions
void returnToHome() {
  logMessage("INFO", "Returning to home positions...");
  startMoveStepper(HOME_THETA1);
  startMoveServo(HOME_THETA2);
}

// Function to start stepper movement
void startMoveStepper(float targetTheta1) {
  Serial.print("[Stepper] Starting move to theta1: ");
  Serial.print(targetTheta1);
  Serial.print(" from currentTheta1: ");
  Serial.println(currentTheta1);
  
  digitalWrite(DIR_PIN, targetTheta1 > currentTheta1 ? HIGH : LOW);
  totalSteps = abs(targetTheta1 - currentTheta1) * 10;
  currentStep = 0;
  movingStepper = true;

  Serial.print("[Stepper] Total Steps to Move: ");
  Serial.println(totalSteps);
}

// Function to handle stepper movement updates
void updateStepper() {
  if (movingStepper && currentStep < totalSteps) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); // Adjust delay for speed
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);
    currentStep++;

    if (currentStep >= totalSteps) {
      movingStepper = false;
      currentTheta1 = (totalSteps / 10.0) * (digitalRead(DIR_PIN) == HIGH ? 1 : -1) + currentTheta1;
      
      Serial.print("[Stepper] Movement Completed, newTheta1: ");
      Serial.println(currentTheta1);
    }
  }
}

// Function to start servo movement
void startMoveServo(float targetTheta2) {
  Serial.print("[Servo] Starting move to theta2: ");
  Serial.print(targetTheta2);
  Serial.print(" from currentTheta2: ");
  Serial.println(currentTheta2);

  targetServoTheta = targetTheta2;
  movingServo = true;
}

// Function to handle servo movement updates
void updateServo() {

  if (abs(currentTheta2 - targetServoTheta) < 0.5) {
    currentTheta2 = targetServoTheta;
    movingServo = false;

    Serial.print("[Servo] Movement Completed, newTheta2: ");
    Serial.println(currentTheta2);      
    Serial.println(movingServo);     
    Serial.println(movingStepper);       
  }

  if (movingServo && currentTheta2 != targetServoTheta) {
    currentTheta2 += (currentTheta2 < targetServoTheta) ? 0.5 : -0.5;
    servoTheta2.write(currentTheta2);
    delay(20);

    Serial.print("[Servo] currentTheta2: ");
    Serial.println(currentTheta2);

    Serial.print("[Servo] targetServoTheta: ");
    Serial.println(targetServoTheta);
  }
}

// Function to move the pen
void movePen(bool penState) {
  penServo.write(penState ? 180 : 0);
  if (penState) {
    logMessage("INFO", "Pen moved down.");
  } else {
    logMessage("INFO", "Pen moved up.");
  }
}

// Function to start both stepper and servo movements
void startMoveTo(float targetTheta1, float targetTheta2, bool penState) {
  Serial.print("[Movement] Starting move to theta1: ");
  Serial.print(targetTheta1);
  Serial.print(", theta2: ");
  Serial.print(targetTheta2);
  Serial.print(", penState: ");
  Serial.println(penState ? "true" : "false");

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
    case WStype_TEXT:
      Serial.print("[WebSocket] Received message: ");
      Serial.println((char*)payload);
      handleReceivedMessage(payload, length);
      break;
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

    Serial.print("[Queue] Queued movement - Theta1: ");
    Serial.print(theta1);
    Serial.print(", Theta2: ");
    Serial.print(theta2);
    Serial.print(", Pen: ");
    Serial.println(pen ? "true" : "false");
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
    Serial.print(".");
  }
  logMessage("INFO", "WiFi Connected.");

  logMessage("INFO", "Initializing WebSocket connection...");
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  // Call WebSocket loop
  webSocket.loop();
  


  // Check if the motors are not moving
  if (!movingStepper && !movingServo) {
    
    // If there are commands in the queue, process the next one
    if (!commandQueue.empty()) {
      MovementCommand cmd = commandQueue.front();
      commandQueue.pop();
      logMessage("INFO", "Processing next queued movement...");
      startMoveTo(cmd.theta1, cmd.theta2, cmd.penState);
      
    } else {
      // No more commands, return to home
      if (currentTheta1 != HOME_THETA1 || currentTheta2 != HOME_THETA2) {
        logMessage("INFO", "No more commands, returning to home position...");
        returnToHome();
      } else {
        logMessage("INFO", "Already at home position, nothing to do.");
      }
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

