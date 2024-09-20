import React, { useState, useEffect } from "react";

const App: React.FC = () => {
  const [status, setStatus] = useState<string>("Not connected");
  const [messages, setMessages] = useState<string[]>([]);
  const [inputMessage, setInputMessage] = useState<string>("");
  const [socket, setSocket] = useState<WebSocket | null>(null);

  useEffect(() => {
    // Create a new WebSocket instance
    const ws = new WebSocket("ws://localhost:8080");

    // Set the WebSocket event handlers
    ws.onopen = () => {
      console.log("Connected to WebSocket server");
      setStatus("Connected");
      ws.send("CLIENT"); // Send initial message identifying the client
    };

    ws.onmessage = (event: MessageEvent) => {
      console.log("Message from server:", event.data);
      setMessages((prevMessages) => [...prevMessages, event.data]);
    };

    ws.onclose = () => {
      console.log("WebSocket connection closed");
      setStatus("Disconnected");
    };

    ws.onerror = (error: Event) => {
      console.error("WebSocket error:", error);
      setStatus("Error");
    };

    // Store WebSocket instance in state
    setSocket(ws);

    // Clean up WebSocket connection on component unmount
    return () => {
      ws.close();
    };
  }, []);

  // Function to send a message to the WebSocket server
  const sendMessage = () => {
    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.send(inputMessage);
      console.log("Sent message:", inputMessage);
      setInputMessage(""); // Clear input field after sending
    } else {
      console.log("WebSocket is not connected");
    }
  };

  return (
    <div className="App">
      <h1>WebSocket Client Portal (React + TypeScript + Vite)</h1>
      <p>
        Status: <strong>{status}</strong>
      </p>

      <h2>Messages</h2>
      <div>
        {messages.map((msg, index) => (
          <p key={index}>{msg}</p>
        ))}
      </div>

      <h2>Send a Message</h2>
      <input
        type="text"
        value={inputMessage}
        onChange={(e) => setInputMessage(e.target.value)}
        placeholder="Enter a message"
      />
      <button onClick={sendMessage}>Send</button>
    </div>
  );
};

export default App;
