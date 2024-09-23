#include <WiFi.h>
#include <WebSocketsClient.h>

const char* ssid = "YOUR_SSID";        // Replace with your Wi-Fi network name
const char* password = "YOUR_PASSWORD"; // Replace with your Wi-Fi password

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;

    case WStype_CONNECTED:
      Serial.println("WebSocket Connected");
      webSocket.sendTXT("ARM"); // Send an initial identification message as "ARM"
      break;

    case WStype_TEXT:
      Serial.printf("Message from server: %s\n", payload);
      break;

    case WStype_BIN:
      Serial.println("Binary message received (not supported)");
      break;

    case WStype_ERROR:
      Serial.println("WebSocket Error");
      break;

    case WStype_PING:
      Serial.println("WebSocket Ping");
      break;

    case WStype_PONG:
      Serial.println("WebSocket Pong");
      break;
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");

  // Initialize WebSocket
  webSocket.begin("192.168.1.100", 8080, "/"); // Use your server's local IP and port
  webSocket.onEvent(webSocketEvent);

  // Connect to the WebSocket server
  webSocket.setReconnectInterval(5000);  // Attempt to reconnect every 5 seconds if disconnected
}

void loop() {
  // Handle WebSocket communication
  webSocket.loop();

  // Send messages periodically (or based on some condition)
  if (WiFi.status() == WL_CONNECTED && webSocket.isConnected()) {
    // Send an array of characters (replace with the actual data)
    String message = "[\"A\", \"B\", \"C\"]";  // JSON format array
    webSocket.sendTXT(message);
    Serial.printf("Sent message: %s\n", message.c_str());
    delay(10000); // Send message every 10 seconds (for example)
  }
}
