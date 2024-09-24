#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

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

WebSocketsClient webSocket;

// Function to control stepper motor via motor driver
void moveStepper(float targetTheta1)
{
  Serial.println("Moving stepper motor...");
  
  // Determine the direction to move (forward or backward)
  if (targetTheta1 > currentTheta1)
  {
    digitalWrite(DIR_PIN, HIGH); // Forward
    Serial.println("Stepper motor moving forward.");
  }
  else
  {
    digitalWrite(DIR_PIN, LOW); // Backward
    Serial.println("Stepper motor moving backward.");
  }

  // Calculate how many steps to take (difference between current and target angles)
  int steps = abs(targetTheta1 - currentTheta1) * 10; // Adjust based on your stepper's steps per degree
  Serial.printf("Number of steps: %d\n", steps);

  // Send pulses to STEP_PIN to move the motor
  for (int i = 0; i < steps; i++)
  {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); // Adjust delay for speed (e.g., 1000 µs = 1 ms)
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);

    // Add yield() to allow the system to handle background tasks and avoid WDT resets
    if (i % 50 == 0)
    { // Yield every 50 steps to avoid long blocking
      yield();
    }
  }

  // Update currentTheta1 to reflect the new position
  currentTheta1 = targetTheta1;
  Serial.printf("Stepper motor moved to: %f\n", currentTheta1);
}

// Pen movement: true = down, false = up
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

// Smooth movement function for stepper (theta1) and servo (theta2)
void moveTo(float targetTheta1, float targetTheta2, bool penState)
{
  Serial.printf("Moving to theta1: %f, theta2: %f, penState: %s\n", targetTheta1, targetTheta2, penState ? "true" : "false");

  // Move pen first
  movePen(penState);

  // Move theta1 (stepper)
  moveStepper(targetTheta1);

  // Smoothly move theta2 (servo)
  while (currentTheta2 != targetTheta2)
  {
    if (currentTheta2 < targetTheta2)
    {
      currentTheta2 += 0.5;
      servoTheta2.write(currentTheta2); // Gradually update servo position
    }
    else if (currentTheta2 > targetTheta2)
    {
      currentTheta2 -= 0.5;
      servoTheta2.write(currentTheta2); // Gradually update servo position
    }

    delay(20); // Small delay for smoother movement
  }
  Serial.printf("Movement completed. Final positions - Theta1: %f, Theta2: %f\n", targetTheta1, targetTheta2);
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
  webSocket.loop();
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

// Function to handle the received WebSocket message
void handleReceivedMessage(uint8_t *payload, size_t length)
{
  Serial.println("Processing received WebSocket message...");
  
  // Parse the incoming JSON message
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

    Serial.printf("Parsed data - Theta1: %f, Theta2: %f, Pen: %s\n", theta1, theta2, pen ? "true" : "false");

    // Perform the movement
    moveTo(theta1, theta2, pen);
  }
}
