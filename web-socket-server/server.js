const WebSocket = require("ws");
const wss = new WebSocket.Server({ port: 8080 });

// Kinematics calculation function
function calculateAngles(x, y) {
  const L1 = 10; // Length of the first arm link
  const L2 = 10; // Length of the second arm link

  const r = Math.sqrt(x ** 2 + y ** 2); // Distance to point

  // Handle unreachable points
  if (r > L1 + L2) {
    return { theta1: NaN, theta2: NaN };
  }

  const cosTheta2 = (r ** 2 - L1 ** 2 - L2 ** 2) / (2 * L1 * L2);

  // Clamp cosTheta2 to the range [-1, 1] to avoid NaN from Math.acos
  const clampedCosTheta2 = Math.max(-1, Math.min(1, cosTheta2));

  const theta2 = Math.acos(clampedCosTheta2); // Elbow angle

  const theta1 =
    Math.atan2(y, x) -
    Math.atan2(L2 * Math.sin(theta2), L1 + L2 * Math.cos(theta2)); // Shoulder angle

  return {
    theta1: (theta1 * 180) / Math.PI, // Convert to degrees
    theta2: (theta2 * 180) / Math.PI,
  };
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

      drawingData.forEach((line) => {
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
