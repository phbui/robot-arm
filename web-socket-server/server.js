const WebSocket = require("ws");
const robot_arm_angles = require("../path-maker/robot_arm_angles.json"); // Adjust the path if needed

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

    try {
      // Try to parse the message as JSON (expecting an id and data)
      const messageData = JSON.parse(message);

      if (messageData.id === "CLIENT" && Array.isArray(messageData.data)) {
        const charArray = messageData.data;

        // Process each character in the array
        charArray.forEach((char) => {
          if (robot_arm_angles[char]) {
            console.log(`Character: ${char}`);
            console.log(robot_arm_angles[char]);
          } else {
            console.log(`Character ${char} not found in robot_arm_angles`);
          }
        });

        // Send a confirmation message back to the client
        ws.send(`Received and processed ${charArray.length} characters`);
      } else if (messageData.id === "ARM") {
        console.log("Message received from ARM.");
        ws.send("Client identified.");
      } else {
        ws.send("Invalid message format or missing data.");
      }
    } catch (error) {
      // If the message is not valid JSON
      console.error("Error processing message:", error);
      ws.send("Error processing message: Invalid JSON");
    }
  });

  // Handle client disconnection
  ws.on("close", () => {
    console.log("Client disconnected");
  });
});

console.log("WebSocket server is running on ws://localhost:8080");
