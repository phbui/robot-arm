import React, { useState, useEffect, useRef } from "react";
import "./assets/css/App.css";

const App: React.FC = () => {
  const [status, setStatus] = useState<string>("Not connected");
  const [messages, setMessages] = useState<string[]>([]);
  const [inputMessage, setInputMessage] = useState<string>("");
  const [socket, setSocket] = useState<WebSocket | null>(null);
  const messagesEndRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    // Create a new WebSocket instance
    const ws = new WebSocket("ws://localhost:8080");

    // Set the WebSocket event handlers
    ws.onopen = () => {
      console.log("Connected to WebSocket server");
      setStatus("Connected");
    };

    ws.onmessage = (event: MessageEvent) => {
      console.log("Message from server:", event.data);
      setMessages((prevMessages) => [...prevMessages, event.data]);
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

  // Function to send an array of characters to the WebSocket server
  const sendMessage = () => {
    if (socket && socket.readyState === WebSocket.OPEN) {
      // Split the input message into an array of characters
      const charArray = inputMessage.split("");

      // Send the array of characters to the WebSocket server
      socket.send(JSON.stringify({ id: "CLIENT", data: charArray }));

      console.log("Sent message:", charArray);
      setInputMessage(""); // Clear input field after sending
    } else {
      console.log("WebSocket is not connected");
    }
  };

  const handleKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.key === "Enter") {
      sendMessage();
    }
  };

  // Scroll to the bottom of the message container when messages change
  useEffect(() => {
    if (messagesEndRef.current) {
      messagesEndRef.current.scrollIntoView({ behavior: "smooth" });
    }
  }, [messages]);

  return (
    <div className="app">
      <header className="app-header">
        <h1 className="app-title">WebSocket Client Portal</h1>
        <p className="app-status">
          Status: <strong className={status}>{status}</strong>
        </p>
      </header>

      <section className="app-messages">
        <h2 className="messages-title">Messages</h2>
        <div className="messages-container">
          {messages.length > 0 ? (
            messages.map((msg, index) => <p key={index}>{msg}</p>)
          ) : (
            <p className="no-messages">No messages yet</p>
          )}
          <div ref={messagesEndRef} />
        </div>
      </section>

      <section className="app-send-message">
        <h2 className="send-message-title">Send a Message</h2>
        <div className="send-message-container">
          <input
            className="send-message-input"
            type="text"
            value={inputMessage}
            onChange={(e) => setInputMessage(e.target.value)}
            onKeyDown={handleKeyDown}
            placeholder="Enter a message"
          />
          <button className="send-message-button" onClick={sendMessage}>
            Send
          </button>
        </div>
      </section>
    </div>
  );
};

export default App;
