#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <queue>  // Include queue library for command queue

// Wi-Fi credentials
const char *ssid = "Phi";
const char *password = "lukaspark";

// WebSocket server details
const char *websocket_host = "172.20.10.11";
const int websocket_port = 8080;
const char *websocket_path = "/";

// Motor driver control pins
const int STEP_PIN = 33;
const int DIR_PIN = 25;
const int LED_PIN = 2;  // LED pin to indicate WebSocket connection

WebSocketsClient webSocket;

// Stepper control variables
float currentTheta1 = 0.0;
float targetTheta1 = 0.0;
int totalSteps = 0;
int currentStep = 0;
bool movingStepper = false;
unsigned long lastStepTime = 0;
unsigned long stepInterval = 1000;  // Step interval in microseconds (adjust for speed)

// Command queue to hold movement commands
std::queue<float> commandQueue;

// Function to log messages with timestamps
void logMessage(const char *level, const char *message)
{
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] ");
  Serial.print(level);
  Serial.print(": ");
  Serial.println(message);
}

// Function to start stepper movement
void startMoveStepper(float targetTheta)
{
  if (movingStepper) {
    logMessage("INFO", "Stepper is still moving, adding new command to the queue.");
    commandQueue.push(targetTheta);  // Add new command to queue
    return;
  }

  String message = "[Stepper] Starting move to theta1: " + String(targetTheta) + " from currentTheta1: " + String(currentTheta1);
  logMessage("INFO", message.c_str());

  // Set the direction of movement
  digitalWrite(DIR_PIN, targetTheta > currentTheta1 ? HIGH : LOW);

  // Calculate the number of steps (0.9 degrees per step for NEMA 17)
  totalSteps = abs(targetTheta - currentTheta1) / 0.9; // Correct calculation for 0.9° steps
  currentStep = 0;
  movingStepper = totalSteps > 0;
  lastStepTime = micros();  // Initialize the timer

  message = "[Stepper] Total Steps to Move: " + String(totalSteps);
  logMessage("INFO", message.c_str());
}

// Non-blocking stepper movement function
void updateStepperNonBlocking()
{
  if (movingStepper && currentStep < totalSteps)
  {
    unsigned long currentTime = micros();
    if (currentTime - lastStepTime >= stepInterval)
    {
      lastStepTime = currentTime;  // Update the last step time

      // Generate a step pulse
      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(10);  // Ensure pulse is wide enough (increase if needed)
      digitalWrite(STEP_PIN, LOW);

      // Increment step count
      currentStep++;

      // Check if the movement is complete
      if (currentStep >= totalSteps)
      {
        movingStepper = false;
        currentTheta1 += totalSteps * (digitalRead(DIR_PIN) == HIGH ? 0.9 : -0.9);  // Update theta based on 0.9 degree steps

        logMessage("INFO", String("[Stepper] Movement Completed, newTheta1: " + String(currentTheta1)).c_str());

        // Process the next command in the queue, if any
        if (!commandQueue.empty()) {
          float nextTheta = commandQueue.front();
          commandQueue.pop();
          startMoveStepper(nextTheta);
        }
      }
    }
  }
}

// WebSocket event handler
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
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
    {
      String message = "[WebSocket] Received message: " + String((char *)payload);
      logMessage("INFO", message.c_str());
      handleReceivedMessage(payload, length);  // Process received command
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
void handleReceivedMessage(uint8_t *payload, size_t length)
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    logMessage("ERROR", "Failed to deserialize JSON.");
    return;
  }

  const char *id = doc["id"];
  if (strcmp(id, "ARM") == 0)
  {
    JsonObject data = doc["data"];
    float theta1 = data["theta1"];

    // Start stepper movement for theta1 (queue if already moving)
    startMoveStepper(theta1);
  }
}

void setup()
{
  Serial.begin(115200);

  // Initialize pins
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  logMessage("INFO", "Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    logMessage("DEBUG", "Waiting for WiFi connection...");
  }
  logMessage("INFO", "WiFi Connected.");

  logMessage("INFO", "Initializing WebSocket connection...");
  webSocket.begin(websocket_host, websocket_port, websocket_path);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop()
{
  webSocket.loop();

  // Continuously update the stepper movement without blocking
  if (movingStepper)
  {
    updateStepperNonBlocking();
  }
}
