const WebSocket = require("ws");
const wss = new WebSocket.Server({ port: 8080 });

const l1x = 7;
const l1y = 1;

const l2x = 5;
const l2y = 0.5;

// Calculate the real lengths of the links
const l1z = Math.sqrt(l1x * l1x + l1y * l1y);
const l2z = Math.sqrt(l2x * l2x + l2y * l2y);

// Define the workspace limits
const rMin = Math.abs(l1z - l2z); // Minimum reach of the arm
const rMax = l1z + l2z; // Maximum reach of the arm

// Helper function to round to 2 decimal places
function roundToTwoDecimals(value) {
  return Math.round(value * 100) / 100;
}

// Function to apply bounds to a point (x, y)
function applyBoundsToPoint(x, y) {
  // Convert to polar coordinates
  const r = Math.sqrt(x * x + y * y);
  const theta = Math.atan2(y, x);

  // Clamp the radius to be within the robot arm's limits
  const rNew = Math.max(rMin, Math.min(r, rMax));

  // Convert back to Cartesian coordinates and round to 2 decimal places
  const xNew = roundToTwoDecimals(rNew * Math.cos(theta));
  const yNew = roundToTwoDecimals(rNew * Math.sin(theta));

  return { x: xNew, y: yNew };
}

// Kinematics calculation function
function calculateAngles(x, y) {
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
  let theta2 = Math.acos(Math.max(-1, Math.min(1, cosTheta2))); // Clamp value to avoid NaN

  const sinTheta2 = Math.sqrt(1 - cosTheta2 * cosTheta2); // Trigonometric identity
  let theta1 = alpha - Math.atan2(l2z * sinTheta2, l1z + l2z * cosTheta2);

  // Convert from radians to degrees and round to 2 decimal places
  let theta1Deg = roundToTwoDecimals(theta1 * (180 / Math.PI));
  let theta2Deg = roundToTwoDecimals(theta2 * (180 / Math.PI));

  // Check if the angles are NaN and handle gracefully
  if (isNaN(theta1Deg)) {
    // For theta1, we can use a reasonable default or clamp to a valid range.
    theta1Deg = 0; // Default to 0 if NaN
  }

  if (isNaN(theta2Deg)) {
    // For theta2, clamp it to the nearest valid value (0 or 180 degrees)
    theta2Deg = 180; // Clamp to max limit if NaN
  }

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

            // Apply bounds to the point and round to 2 decimal places
            const { x: boundedX, y: boundedY } = applyBoundsToPoint(x, y);

            // Calculate the angles for the robot arm and round to 2 decimal places
            const angles = calculateAngles(boundedX, boundedY);
            console.log(
              `Coordinates: (${boundedX}, ${boundedY}) => Angles: ${angles.theta1}, ${angles.theta2}`
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
                `Calculated angles for (${boundedX}, ${boundedY})`,
                {
                  theta1: angles.theta1,
                  theta2: angles.theta2,
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
