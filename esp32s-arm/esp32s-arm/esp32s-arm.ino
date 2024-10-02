#include <WiFi.h>              // WiFi library to handle connection
#include <WebSocketsClient.h>   // WebSockets library to communicate with a server
#include <ArduinoJson.h>        // ArduinoJson library to parse JSON data
#include <queue>                // Standard library for queue data structure
#include <ESP32Servo.h>         // Library to control servos on the ESP32

// WiFi credentials
const char *ssid = "Phi";              // WiFi SSID
const char *password = "lukaspark";    // WiFi password

// WebSocket connection details
const char *websocket_host = "172.20.10.11";   // WebSocket server IP address
const int websocket_port = 8080;               // WebSocket server port
const char *websocket_path = "/";              // WebSocket connection path

// Pin assignments for stepper motor, servos, and LEDs
const int STEP_PIN = 33;
const int DIR_PIN = 25;
const int SERVO_PIN = 19;
const int PEN_PIN = 18;
const int LED_PIN = 2;

// Declare WebSocket and servo objects
WebSocketsClient webSocket;   // WebSocket client
Servo servoTheta2;            // Servo for theta2 angle control
Servo penServo;               // Servo for pen control

// Variables for managing movement states and commands
float currentTheta1 = 0.0;          // Current position of stepper motor
float targetTheta1 = 0.0;           // Target position for stepper motor
float currentTheta2 = 180.0;        // Current position of servo
bool penState = false;              // Current state of the pen (up or down)
int totalSteps = 0;                 // Total steps required for stepper motor to reach target
int currentStep = 0;                // Current step progress
bool movingStepper = false;         // Stepper movement state
bool movingServo = false;           // Servo movement state
unsigned long lastStepTime = 0;     // Last timestamp when stepper moved
unsigned long stepInterval = 1000;  // Time interval between each step
unsigned long servoMoveStartTime = 0;  // Timestamp when servo movement started
unsigned long servoMoveDuration = 0;   // Duration of servo movement

const float timePerDegree = 3.33;   // Time taken for servo to move per degree

// Structure to hold movement commands
struct MovementCommand {
  float theta1;
  float theta2;
  bool pen;
};

// Queue to store movement commands
std::queue<MovementCommand> commandQueue;

// Function to log messages to the serial monitor
void logMessage(const char *level, const char *message) {
  Serial.print("[");
  Serial.print(millis());  // Log the current time in milliseconds
  Serial.print("] ");
  Serial.print(level);      // Log the message level (INFO, ERROR, etc.)
  Serial.print(": ");
  Serial.println(message);  // Log the message itself
}

// Function to move the pen up or down
void movePen(bool state) {
  penServo.write(state ? 0 : 180);  // Write 0 degrees to lower the pen, 180 degrees to raise it
  penState = state;                 // Update pen state
}

// Function to initiate movement to the target positions (theta1, theta2) and pen state
void startMoveTo(float targetTheta1, float targetTheta2, bool pen) {
  if (movingStepper || movingServo) {  // Check if a movement is already in progress
    logMessage("INFO", "Motors are still moving, adding new command to the queue.");
    commandQueue.push({targetTheta1, targetTheta2, pen});  // Add new command to the queue
    return;
  }

  movePen(pen);  // Move the pen to the specified state

  // Set direction pin for stepper motor based on target theta1
  digitalWrite(DIR_PIN, targetTheta1 > currentTheta1 ? HIGH : LOW);
  logMessage("INFO", String("[Movement] Starting move to theta1: " + String(targetTheta1) + ", theta2: " + String(targetTheta2)).c_str());

  // Calculate total steps for stepper motor movement
  totalSteps = abs(targetTheta1 - currentTheta1) / 1.8;  // 1.8 degrees per step
  currentStep = 0;
  movingStepper = totalSteps > 0;  // Set stepper movement flag if steps are required
  lastStepTime = micros();         // Set initial timestamp for stepping

  // Calculate servo movement duration based on angle difference
  float angleDifference = abs(targetTheta2 - currentTheta2);
  servoMoveDuration = angleDifference * timePerDegree;
  currentTheta2 = targetTheta2;   // Update target position for servo
  servoTheta2.write(currentTheta2);  // Move servo to target position
  movingServo = true;             // Set servo movement flag
  logMessage("INFO", String("[Servo] Moving to theta2: " + String(currentTheta2) + " over " + String(servoMoveDuration) + " ms").c_str());
  servoMoveStartTime = millis();  // Set timestamp for servo movement start
}

// Non-blocking stepper motor update function
void updateStepperNonBlocking() {
  if (movingStepper && currentStep < totalSteps) {  // Check if the stepper is still moving
    unsigned long currentTime = micros();  // Get the current time
    if (currentTime - lastStepTime >= stepInterval) {  // Check if it's time for the next step
      lastStepTime = currentTime;  // Update the last step time

      // Step the motor
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(STEP_PIN, LOW);

      currentStep++;  // Increment step counter

      // Check if stepper movement is complete
      if (currentStep >= totalSteps) {
        movingStepper = false;  // Reset stepper movement flag
        currentTheta1 += totalSteps * (digitalRead(DIR_PIN) == HIGH ? 1.8 : -1.8);  // Update current position
        logMessage("INFO", String("[Stepper] Movement Completed, newTheta1: " + String(currentTheta1)).c_str());
      }
    }
  }
}

// Non-blocking servo update function
void updateServoNonBlocking() {
  if (movingServo && millis() - servoMoveStartTime >= servoMoveDuration) {  // Check if servo movement is complete
    movingServo = false;  // Reset servo movement flag
    logMessage("INFO", "[Servo] Movement Completed");
  }
}

// Function to check if all movements are completed and process queued commands
void checkMovementCompletion() {
  if (!movingStepper && !movingServo && !commandQueue.empty()) {  // Check if motors are idle and there are queued commands
    MovementCommand nextCommand = commandQueue.front();  // Get the next command from the queue
    commandQueue.pop();  // Remove the command from the queue
    logMessage("INFO", "[Movement] All Movements Completed");
    startMoveTo(nextCommand.theta1, nextCommand.theta2, nextCommand.pen);  // Start the next movement
  }
}

// WebSocket event handler function
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  String message;

  switch (type) {
    case WStype_DISCONNECTED:
      logMessage("ERROR", "Disconnected from server.");
      digitalWrite(LED_PIN, LOW);  // Turn off LED on disconnect
      break;
    case WStype_CONNECTED:
      logMessage("INFO", "Connected to server.");
      webSocket.sendTXT("{\"id\": \"ARM\"}");  // Send identification message
      digitalWrite(LED_PIN, HIGH);  // Turn on LED on successful connection
      break;
    case WStype_TEXT:
      message = "[WebSocket] Received message: " + String((char *)payload);
      logMessage("INFO", message.c_str());
      handleReceivedMessage(payload, length);  // Handle received WebSocket message
      break;
    case WStype_ERROR:
      logMessage("ERROR", "WebSocket Error occurred.");
      break;
  }
}

// Function to handle received WebSocket messages
void handleReceivedMessage(uint8_t *payload, size_t length) {
  StaticJsonDocument<1024> doc;  // Create a JSON document to store the message
  DeserializationError error = deserializeJson(doc, payload);  // Deserialize the JSON payload

  if (error) {  // Check if JSON deserialization failed
    logMessage("ERROR", "Failed to deserialize JSON.");
    return;
  }

  // Extract data from the JSON message
  const char *id = doc["id"];
  if (strcmp(id, "ARM") == 0) {
    JsonObject data = doc["data"];
    float theta1 = data["theta1"];
    float theta2 = data["theta2"];
    bool pen = data["pen"];

    // Start movement based on the received data
    startMoveTo(theta1, theta2, pen);
  }
}

// Arduino setup function
void setup() {
  Serial.begin(115200);  // Initialize serial communication

  // Initialize pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  // Turn off LED initially

  // Attach servos
  servoTheta2.attach(SERVO_PIN);
  penServo.attach(PEN_PIN);

  // Connect to WiFi
  logMessage("INFO", "Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    logMessage("DEBUG", "Waiting for WiFi connection...");
  }
  logMessage("INFO", "WiFi Connected.");

  // Initialize WebSocket connection
  logMessage("INFO", "Initializing WebSocket connection...");
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);  // Set WebSocket event handler
  webSocket.setReconnectInterval(5000);  // Set auto-reconnect interval
}

// Arduino main loop function
void loop() {
  webSocket.loop();  // Handle WebSocket events

  // Update stepper and servo movements if they are active
  if (movingStepper) {
    updateStepperNonBlocking();
  }

  if (movingServo) {
    updateServoNonBlocking();
  }

  checkMovementCompletion();  // Check if movement is complete and process the next command
}
