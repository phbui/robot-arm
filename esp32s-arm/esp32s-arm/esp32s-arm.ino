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
const int STEP_PIN = 8;    // Pin to send step pulses to the motor driver
const int DIR_PIN = 9;     // Pin to set direction of stepper motor rotation
const int SERVO_PIN = 6;
const int PEN_PIN = 7;

// Servo control for theta2 and pen
Servo servoTheta2;
Servo penServo;

// Variables for current positions
float currentTheta1 = 0.0;
float currentTheta2 = 0.0;

WebSocketsClient webSocket;

// Function to control stepper motor via motor driver
void moveStepper(float targetTheta1) {
  // Determine the direction to move (forward or backward)
  if (targetTheta1 > currentTheta1) {
    digitalWrite(DIR_PIN, HIGH); // Forward
  } else {
    digitalWrite(DIR_PIN, LOW);  // Backward
  }

  // Calculate how many steps to take (difference between current and target angles)
  int steps = abs(targetTheta1 - currentTheta1) * 10; // Adjust based on your stepper's steps per degree

  // Send pulses to STEP_PIN to move the motor
  for (int i = 0; i < steps; i++) {
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(1000); // Adjust delay for speed (e.g., 1000 µs = 1 ms)
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(1000);

    // Add yield() to allow the system to handle background tasks and avoid WDT resets
    if (i % 50 == 0) { // Yield every 50 steps to avoid long blocking
      yield();
    }
  }

  // Update currentTheta1 to reflect the new position
  currentTheta1 = targetTheta1;
}

// Pen movement: true = down, false = up
void movePen(bool penState) {
  if (penState) {
    penServo.write(180);  // Move pen down
  } else {
    penServo.write(0);    // Move pen up
  }
}

// Smooth movement function for stepper (theta1) and servo (theta2)
void moveTo(float targetTheta1, float targetTheta2, bool penState) {
  // Move pen first
  movePen(penState);

  // Move theta1 (stepper)
  moveStepper(targetTheta1);

  // Smoothly move theta2 (servo)
  while (currentTheta2 != targetTheta2) {
    if (currentTheta2 < targetTheta2) {
      currentTheta2 += 0.5;
      servoTheta2.write(currentTheta2); // Gradually update servo position
    } else if (currentTheta2 > targetTheta2) {
      currentTheta2 -= 0.5;
      servoTheta2.write(currentTheta2); // Gradually update servo position
    }

    delay(20); // Small delay for smoother movement
  }
}

void setup()
{
  Serial.begin(115200);

  // Pin modes for motor driver control
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);

  // Attach servos for theta2 and pen control using ESP32Servo
  servoTheta2.attach(SERVO_PIN);  // Attach theta2 servo to pin 6
  penServo.attach(PEN_PIN);     // Attach pen servo to pin 7

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected to Wi-Fi");

  // Setup WebSocket
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
    Serial.println("Disconnected from WebSocket server");
    break;

  case WStype_CONNECTED:
    Serial.println("Connected to WebSocket server");
    // Send ARM identification message
    webSocket.sendTXT("{\"id\": \"ARM\"}");
    break;

  case WStype_TEXT:
    Serial.printf("Received message: %s\n", payload);
    handleReceivedMessage(payload, length); // Handle the received message
    break;

  case WStype_ERROR:
    Serial.println("Error occurred");
    break;

  default:
    break;
  }
}

// Function to handle the received WebSocket message
void handleReceivedMessage(uint8_t *payload, size_t length)
{
  // Parse the incoming JSON message
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.f_str());
    return;
  }

  const char *id = doc["id"];
  JsonArray data = doc["data"];

  // Iterate through each movement in the data array
  for (JsonObject movement : data)
  {
    float theta1 = movement["theta1"];
    float theta2 = movement["theta2"];
    bool pen = movement["pen"];

    Serial.printf("Moving to theta1: %f, theta2: %f, pen: %s\n", theta1, theta2, pen ? "true" : "false");

    // Perform the movement
    moveTo(theta1, theta2, pen);
  }
}
