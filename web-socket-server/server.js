const WebSocket = require("ws");
const wss = new WebSocket.Server({ port: 8080 });

const l1x = 7;
const l1y = 1;

const l2x = 5;
const l2y = 0.5;

// Kinematics calculation function
function calculateAngles(x, y) {
  // Calculate the real lengths of the links
  const l1z = Math.sqrt(l1x * l1x + l1y * l1y);
  const l2z = Math.sqrt(l2x * l2x + l2y * l2y);

  // Calculate the angles (offsets) for the links
  const O1 = Math.atan2(l1y, l1x);
  const O2 = Math.atan2(l2y, l2x);

  // Adjust the target point for the offsets
  const adjustedX = x - O1; // Compensating for the O1 offset
  const adjustedY = y - O2; // Compensating for the O2 offset

  // Inverse kinematics equations to find the angles
  const r = Math.sqrt(adjustedX * adjustedX + adjustedY * adjustedY);
  const alpha = Math.atan2(adjustedY, adjustedX);

  // Compute angles based on the lengths of L1 and L2
  const cosTheta2 = (r * r - l1z * l1z - l2z * l2z) / (2 * l1z * l2z);
  const theta2 = Math.acos(cosTheta2);

  const sinTheta2 = Math.sqrt(1 - cosTheta2 * cosTheta2); // Trigonometric identity
  const theta1 = alpha - Math.atan2(l2z * sinTheta2, l1z + l2z * cosTheta2);

  // Convert from radians to degrees
  const theta1Deg = theta1 * (180 / Math.PI);
  const theta2Deg = theta2 * (180 / Math.PI);

  return { theta1: theta1Deg, theta2: theta2Deg };
}

let connections = {
  arm: null,
  client: null,
};

// Function to send a structured message to a WebSocket client
function sendMessage(ws, id, message, data = null) {
  ws.send(
    JSON.stringify({
      id,
      message,
      data,
    })
  );
}

wss.on("connection", (ws) => {
  console.log("Client connected");

  ws.on("message", (message) => {
    const messageData = JSON.parse(message);

    // Handle drawing data received from client
    if (messageData.id === "DRAWING" && Array.isArray(messageData.data)) {
      const drawingData = messageData.data;

      drawingData.forEach((line, lineIndex) => {
        line.points.forEach((point, index) => {
          if (index % 2 === 0) {
            const x = line.points[index];
            const y = line.points[index + 1];

            // Calculate the angles for the robot arm
            const angles = calculateAngles(x, y);
            console.log(
              `Coordinates: (${x}, ${y}) => Angles: ${angles.theta1}, ${angles.theta2}`
            );

            // Send the calculated angles to ARM
            if (
              connections.arm &&
              connections.arm.readyState === WebSocket.OPEN
            ) {
              sendMessage(connections.arm, "ARM", "Move to calculated angles", {
                theta1: angles.theta1,
                theta2: angles.theta2,
                pen: true, // Assuming pen down during drawing
              });
            }

            // Send the calculated angles back to the client for logging
            if (
              connections.client &&
              connections.client.readyState === WebSocket.OPEN
            ) {
              sendMessage(
                connections.client,
                "SYSTEM",
                `Calculated angles for (${x}, ${y})`,
                {
                  theta1: angles.theta1.toFixed(2),
                  theta2: angles.theta2.toFixed(2),
                }
              );
            }
          }
        });

        // Send pen up command after the last line in the batch
        if (
          lineIndex === drawingData.length - 1 &&
          connections.arm &&
          connections.arm.readyState === WebSocket.OPEN
        ) {
          sendMessage(connections.arm, "ARM", "Pen up", {
            pen: false, // Lift the pen after the drawing
          });
        }
      });
    } else if (messageData.id === "ARM") {
      // Store the ARM connection
      connections.arm = ws;
      console.log("ARM connected");

      // Notify CLIENT that ARM is connected
      if (
        connections.client &&
        connections.client.readyState === WebSocket.OPEN
      ) {
        sendMessage(connections.client, "SYSTEM", "ARM connected");
      }
    }
  });

  ws.on("close", () => {
    // Handle ARM disconnection
    if (connections.arm === ws) {
      connections.arm = null;
      console.log("ARM disconnected");

      // Notify CLIENT that ARM is disconnected
      if (
        connections.client &&
        connections.client.readyState === WebSocket.OPEN
      ) {
        sendMessage(connections.client, "SYSTEM", "ARM disconnected");
      }
    }

    // Handle CLIENT disconnection
    if (connections.client === ws) {
      connections.client = null;
      console.log("Client disconnected");
    }
  });

  // Store the client connection and send an initial status message
  if (!connections.client) {
    connections.client = ws;
    console.log("Client connection established");

    // Send a system message to the client indicating successful connection
    sendMessage(connections.client, "SYSTEM", "Client connected");

    // Notify CLIENT if ARM is already connected
    if (connections.arm && connections.arm.readyState === WebSocket.OPEN) {
      sendMessage(connections.client, "SYSTEM", "ARM connected");
    }
  }
});

console.log("WebSocket server is running on ws://localhost:8080");
