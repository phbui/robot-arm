const WebSocket = require("ws");
const robot_arm_angles = require("../path-maker/robot_arm_angles.json");

const wss = new WebSocket.Server({ port: 8080 });

let connections = {
  arm: null, // Store the WebSocket connection of the ARM
  client: null, // Store the WebSocket connection of the CLIENT
};

wss.on("connection", (ws) => {
  console.log("Client connected");

  ws.on("message", (message) => {
    console.log(`Received message: ${message}`);

    try {
      const messageData = JSON.parse(message);

      if (messageData.id === "CLIENT" && Array.isArray(messageData.data)) {
        // Store the CLIENT connection
        connections.client = ws;

        // Check if ARM is already connected
        if (connections.arm) {
          // Notify CLIENT that ARM is connected
          ws.send(JSON.stringify({ id: "SYSTEM", message: "ARM connected" }));
        }

        const charArray = messageData.data;

        charArray.forEach((char) => {
          if (robot_arm_angles[char]) {
            console.log(`Character: ${char}`);
            console.log(robot_arm_angles[char]);

            // Prepare the message to send to the ARM
            const messageToArm = JSON.stringify({
              id: "ARM",
              character: char,
              data: robot_arm_angles[char],
            });

            // Send the processed character data to ARM (ESP32)
            if (
              connections.arm &&
              connections.arm.readyState === WebSocket.OPEN
            ) {
              connections.arm.send(messageToArm);

              // Write back to CLIENT what was sent to ARM
              if (
                connections.client &&
                connections.client.readyState === WebSocket.OPEN
              ) {
                connections.client.send(`Sent to ARM: ${messageToArm}`);
              }
            }
          } else {
            console.log(`Character ${char} not found in robot_arm_angles`);
          }
        });

        ws.send(`Received and processed ${charArray.length} characters`);
      } else if (messageData.id === "ARM") {
        console.log("Message received from ARM.");

        // Store the ARM connection
        connections.arm = ws;

        // Notify all clients that the ARM is connected
        wss.clients.forEach((client) => {
          if (client !== ws && client.readyState === WebSocket.OPEN) {
            client.send(
              JSON.stringify({ id: "SYSTEM", message: "ARM connected" })
            );
          }
        });

        ws.send("Client identified.");
      } else {
        ws.send("Invalid message format or missing data.");
      }
    } catch (error) {
      console.error("Error processing message:", error);
      ws.send("Error processing message: Invalid JSON");
    }
  });

  ws.on("close", () => {
    console.log("Client disconnected");

    if (connections.arm === ws) {
      // If the ARM disconnected
      console.log("ARM disconnected");
      connections.arm = null;

      // Notify all clients that the ARM has disconnected
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(
            JSON.stringify({ id: "SYSTEM", message: "ARM disconnected" })
          );
        }
      });
    } else if (connections.client === ws) {
      // If the CLIENT disconnected
      console.log("CLIENT disconnected");
      connections.client = null;
    }
  });
});

console.log("WebSocket server is running on ws://localhost:8080");
