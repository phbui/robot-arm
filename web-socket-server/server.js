const WebSocket = require("ws");

// Create a WebSocket server on port 8080
const wss = new WebSocket.Server({ port: 8080 });

// Handle new WebSocket connections
wss.on("connection", (ws) => {
  console.log("Client connected");

  // Send a message to the client when they connect
  ws.send("Hello from the WebSocket server!");

  // Handle incoming messages from the client
  ws.on("message", (message) => {
    console.log(`Received message: ${message}`);
    // Echo the received message back to the client
    ws.send(`You said: ${message}`);
  });

  // Handle client disconnection
  ws.on("close", () => {
    console.log("Client disconnected");
  });
});

console.log("WebSocket server is running on ws://localhost:8080");
