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
const int STEP_PIN = 12; // Pin to send step pulses to the motor driver
const int DIR_PIN = 13;  // Pin to set direction of stepper motor rotation
const int SERVO_PIN = 14;
const int PEN_PIN = 15;

// LED pin (Blue light to indicate WebSocket connection)
const int LED_PIN = 2; // Onboard LED for many ESP32 boards, use another pin if using an external LED

// Servo control for theta2 and pen
Servo servoTheta2;
Servo penServo;

// Variables for current positions
float currentTheta1 = 0.0;
float currentTheta2 = 0.0;

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

// Non-blocking stepper movement
void startMoveStepper(float targetTheta1)
{
  // Determine the direction to move (forward or backward)
  if (targetTheta1 > currentTheta1)
  {
    digitalWrite(DIR_PIN, HIGH); // Forward
  }
  else
  {
    digitalWrite(DIR_PIN, LOW); // Backward
  }

  // Calculate the total number of steps to move
  totalSteps = abs(targetTheta1 - currentTheta1) * 10; // Adjust based on your stepper's steps per degree
  currentStep = 0; // Reset step counter
  movingStepper = true; // Indicate movement is in progress
}

void updateStepper()
{
  if (movingStepper && currentStep < totalSteps)
  {
    // Perform a single step
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); // Adjust delay for speed (e.g., 1000 µs = 1 ms)
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);

    currentStep++; // Increment the step count

    // Add yield() to avoid WDT resets during long movements
    if (currentStep % 50 == 0)
    {
      yield();
    }

    if (currentStep >= totalSteps)
    {
      movingStepper = false; // Movement completed
      currentTheta1 = (totalSteps / 10.0) + currentTheta1; // Update to the new theta position
      Serial.printf("Stepper motor moved to: %f\n", currentTheta1);
    }
  }
}

// Variables for non-blocking servo movement
float targetServoTheta = 0.0;

void startMoveServo(float targetTheta2)
{
  targetServoTheta = targetTheta2; // Set the target angle for the servo
  movingServo = true; // Indicate movement is in progress
}

void updateServo()
{
  if (movingServo && currentTheta2 != targetServoTheta)
  {
    // Move the servo gradually toward the target position
    if (currentTheta2 < targetServoTheta)
    {
      currentTheta2 += 0.5;
    }
    else if (currentTheta2 > targetServoTheta)
    {
      currentTheta2 -= 0.5;
    }

    servoTheta2.write(currentTheta2); // Update servo position

    delay(20); // Small delay for smoother movement

    // Check if the movement is complete
    if (abs(currentTheta2 - targetServoTheta) < 0.5)
    {
      currentTheta2 = targetServoTheta;
      movingServo = false; // Movement completed
      Serial.printf("Servo moved to: %f\n", currentTheta2);
    }
  }
}

void setup()
{
  Serial.begin(115200);

  // Pin modes for motor driver control
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  // Setup LED pin for WebSocket connection status
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED off initially

  // Attach servos for theta2 and pen control using ESP32Servo
  servoTheta2.attach(SERVO_PIN);
  penServo.attach(PEN_PIN);

  // Connect to Wi-Fi
  Serial.println("Attempting to connect to Wi-Fi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to Wi-Fi");

  // Setup WebSocket
  Serial.println("Initializing WebSocket connection...");
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);

  // Attempt to identify as ARM on connection
  webSocket.setReconnectInterval(5000);
}

void loop()
{
  webSocket.loop(); // Continue handling WebSocket communication

  // If no movement is in progress and the command queue is not empty, process the next command
  if (!movingStepper && !movingServo && !commandQueue.empty())
  {
    MovementCommand cmd = commandQueue.front();
    commandQueue.pop();
    startMoveTo(cmd.theta1, cmd.theta2, cmd.penState);
  }

  // Continuously update stepper and servo movements in a non-blocking way
  updateStepper();  // Handle stepper motor movement
  updateServo();    // Handle servo movement
}

void movePen(bool penState)
{
  if (penState)
  {
    penServo.write(180); // Move pen down
    Serial.println("Pen moved down.");
  }
  else
  {
    penServo.write(0); // Move pen up
    Serial.println("Pen moved up.");
  }
}

void startMoveTo(float targetTheta1, float targetTheta2, bool penState)
{
  Serial.printf("Starting move to theta1: %f, theta2: %f, penState: %s\n", targetTheta1, targetTheta2, penState ? "true" : "false");

  // Move pen first
  movePen(penState);

  // Start non-blocking movements
  startMoveStepper(targetTheta1);
  startMoveServo(targetTheta2);
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("Disconnected from WebSocket server.");
    digitalWrite(LED_PIN, LOW); // Turn off LED when disconnected
    break;

  case WStype_CONNECTED:
    Serial.println("Connected to WebSocket server.");
    webSocket.sendTXT("{\"id\": \"ARM\"}");
    digitalWrite(LED_PIN, HIGH); // Turn on LED when connected
    break;

  case WStype_TEXT:
    Serial.printf("Received message: %s\n", payload);
    handleReceivedMessage(payload, length); // Handle the received message
    break;

  case WStype_ERROR:
    Serial.println("Error occurred.");
    break;

  default:
    break;
  }
}

void handleReceivedMessage(uint8_t *payload, size_t length)
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("Failed to deserialize JSON: ");
    Serial.println(error.f_str());
    return;
  }

  const char *id = doc["id"];
  if (strcmp(id, "ARM") == 0)
  {
    JsonObject data = doc["data"];
    float theta1 = data["theta1"];
    float theta2 = data["theta2"];
    bool pen = data["pen"];

    MovementCommand cmd = { theta1, theta2, pen };
    commandQueue.push(cmd);  // Add movement to queue
    Serial.printf("Queued movement - Theta1: %f, Theta2: %f, Pen: %s\n", theta1, theta2, pen ? "true" : "false");
  }
}
