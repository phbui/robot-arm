const WebSocket = require("ws");
const robot_arm_angles = require("../path-maker/robot_arm_angles.json"); // Adjust the path if needed

const wss = new WebSocket.Server({ port: 8080 });

let armConnected = false;

wss.on("connection", (ws) => {
  console.log("Client connected");

  ws.on("message", (message) => {
    console.log(`Received message: ${message}`);

    try {
      const messageData = JSON.parse(message);

      if (messageData.id === "CLIENT" && Array.isArray(messageData.data)) {
        const charArray = messageData.data;

        charArray.forEach((char) => {
          if (robot_arm_angles[char]) {
            console.log(`Character: ${char}`);
            console.log(robot_arm_angles[char]);

            // Send the processed character data to ARM (ESP32)
            wss.clients.forEach((client) => {
              if (client !== ws && client.readyState === WebSocket.OPEN) {
                client.send(
                  JSON.stringify({
                    id: "ARM",
                    character: char,
                    data: robot_arm_angles[char],
                  })
                );
              }
            });
          } else {
            console.log(`Character ${char} not found in robot_arm_angles`);
          }
        });

        ws.send(`Received and processed ${charArray.length} characters`);
      } else if (messageData.id === "ARM") {
        console.log("Message received from ARM.");
        armConnected = true;

        // Notify all clients that the ARM is connected
        wss.clients.forEach((client) => {
          if (client.readyState === WebSocket.OPEN) {
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

    if (armConnected) {
      armConnected = false;

      // Notify all clients that the ARM has disconnected
      wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
          client.send(
            JSON.stringify({ id: "SYSTEM", message: "ARM disconnected" })
          );
        }
      });
    }
  });
});

console.log("WebSocket server is running on ws://localhost:8080");
