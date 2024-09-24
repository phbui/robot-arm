#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "Your_Hotspot_SSID";    // Replace with your phone's hotspot SSID
const char* password = "Your_Hotspot_Password";  // Replace with your phone's hotspot password

const char* websocket_host = "192.168.x.x";  // Replace with your WebSocket server IP
const int websocket_port = 8080;
const char* websocket_path = "/";

WebSocketsClient webSocket;

const int LED_PIN = 2;  // Use GPIO2 for built-in LED

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  
  while (WiFi.status() != WL_CONNECTED) {
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

void loop() {
  webSocket.loop();
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
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
      
      // Blink LED to indicate a message was received
      digitalWrite(LED_PIN, HIGH);
      delay(500);  // 500ms ON
      digitalWrite(LED_PIN, LOW);
      delay(500);  // 500ms OFF
      break;

    case WStype_ERROR:
      Serial.println("Error occurred");
      break;

    default:
      break;
  }
}
