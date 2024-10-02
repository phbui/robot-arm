#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <queue>
#include <ESP32Servo.h>

const char *ssid = "Phi";
const char *password = "lukaspark";
const char *websocket_host = "172.20.10.11";
const int websocket_port = 8080;
const char *websocket_path = "/";

const int STEP_PIN = 33;
const int DIR_PIN = 25;
const int SERVO_PIN = 19;
const int PEN_PIN = 18;
const int LED_PIN = 2;

WebSocketsClient webSocket;
Servo servoTheta2;
Servo penServo;

float currentTheta1 = 0.0;
float targetTheta1 = 0.0;
float currentTheta2 = 180.0;
bool penState = false;
int totalSteps = 0;
int currentStep = 0;
bool movingStepper = false;
bool movingServo = false;
unsigned long lastStepTime = 0;
unsigned long stepInterval = 1000;
unsigned long servoMoveStartTime = 0;
unsigned long servoMoveDuration = 0;

const float timePerDegree = 3.33;

struct MovementCommand {
  float theta1;
  float theta2;
  bool pen;
};
std::queue<MovementCommand> commandQueue;

void logMessage(const char *level, const char *message) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print("] ");
  Serial.print(level);
  Serial.print(": ");
  Serial.println(message);
}

void movePen(bool state) {
  penServo.write(state ? 0 : 180);
  penState = state;
}

void startMoveTo(float targetTheta1, float targetTheta2, bool pen) {
  if (movingStepper || movingServo) {
    logMessage("INFO", "Motors are still moving, adding new command to the queue.");
    commandQueue.push({targetTheta1, targetTheta2, pen});
    return;
  }

movePen(pen);

  digitalWrite(DIR_PIN, targetTheta1 > currentTheta1 ? HIGH : LOW);
  logMessage("INFO", String("[Movement] Starting move to theta1: " + String(targetTheta1) + ", theta2: " + String(targetTheta2)).c_str());

  totalSteps = abs(targetTheta1 - currentTheta1) / 1.8;
  currentStep = 0;
  movingStepper = totalSteps > 0;
  lastStepTime = micros();

  float angleDifference = abs(targetTheta2 - currentTheta2);
  servoMoveDuration = angleDifference * timePerDegree;
  currentTheta2 = targetTheta2;
  servoTheta2.write(currentTheta2);
  movingServo = true;
  logMessage("INFO", String("[Servo] Moving to theta2: " + String(currentTheta2) + " over " + String(servoMoveDuration) + " ms").c_str());
  servoMoveStartTime = millis();


}

void updateStepperNonBlocking() {
  if (movingStepper && currentStep < totalSteps) {
    unsigned long currentTime = micros();
    if (currentTime - lastStepTime >= stepInterval) {
      lastStepTime = currentTime;

      digitalWrite(STEP_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(STEP_PIN, LOW);

      currentStep++;

      if (currentStep >= totalSteps) {
        movingStepper = false;
        currentTheta1 += totalSteps * (digitalRead(DIR_PIN) == HIGH ? 1.8 : -1.8);
        logMessage("INFO", String("[Stepper] Movement Completed, newTheta1: " + String(currentTheta1)).c_str());
      }
    }
  }
}

void updateServoNonBlocking() {
  if (movingServo && millis() - servoMoveStartTime >= servoMoveDuration) { 
    movingServo = false;
    logMessage("INFO", "[Servo] Movement Completed");
  }
}

void checkMovementCompletion() {
  if (!movingStepper && !movingServo && !commandQueue.empty()) {
    MovementCommand nextCommand = commandQueue.front();
    commandQueue.pop();
    logMessage("INFO", "[Movement] All Movements Completed");
    startMoveTo(nextCommand.theta1, nextCommand.theta2, nextCommand.pen);
  }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  String message;

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
      message = "[WebSocket] Received message: " + String((char *)payload);
      logMessage("INFO", message.c_str());
      handleReceivedMessage(payload, length);
      break;
    case WStype_ERROR:
      logMessage("ERROR", "WebSocket Error occurred.");
      break;
  }
}

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

    startMoveTo(theta1, theta2, pen);
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

  if (movingStepper) {
    updateStepperNonBlocking();
  }

  if (movingServo) {
    updateServoNonBlocking();
  }

  checkMovementCompletion();
}
