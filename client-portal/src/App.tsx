import React, { useState, useEffect, useRef } from "react";
import { Stage, Layer, Line } from "react-konva";
import "./assets/css/App.css";

const App: React.FC = () => {
  const [status, setStatus] = useState<string>("Not connected");
  const [armStatus, setArmStatus] = useState<string>("ARM not connected");
  const [messages, setMessages] = useState<string[]>([]);
  const messagesEndRef = useRef<HTMLDivElement>(null);
  const [lines, setLines] = useState<any[]>([]); // Drawing lines
  const isDrawing = useRef(false);
  const [socket, setSocket] = useState<WebSocket | null>(null);

  useEffect(() => {
    const ws = new WebSocket("ws://localhost:8080");

    ws.onopen = () => {
      console.log("Connected to WebSocket server");
      setStatus("Connected");
    };

    ws.onmessage = (event: MessageEvent) => {
      const serverMessage = event.data;
      setMessages((prevMessages) => [...prevMessages, serverMessage]);

      try {
        const parsedMessage = JSON.parse(serverMessage);
        if (parsedMessage.id === "SYSTEM") {
          setArmStatus(parsedMessage.message);
        }
      } catch (error) {
        console.error("Failed to parse message:", error);
      }
    };

    ws.onerror = (error: Event) => {
      console.error("WebSocket error:", error);
      setStatus("Error");
    };

    setSocket(ws);

    return () => {
      ws.close();
    };
  }, []);

  const handleMouseDown = (event: any) => {
    isDrawing.current = true;
    const pos = event.target.getStage().getPointerPosition();
    setLines([...lines, { points: [pos.x, pos.y] }]);
  };

  const handleMouseMove = (event: any) => {
    if (!isDrawing.current) return;
    const stage = event.target.getStage();
    const point = stage.getPointerPosition();
    const lastLine = lines[lines.length - 1];
    lastLine.points = lastLine.points.concat([point.x, point.y]);
    lines.splice(lines.length - 1, 1, lastLine);
    setLines(lines.concat());
  };

  const handleMouseUp = () => {
    isDrawing.current = false;
    if (socket && socket.readyState === WebSocket.OPEN) {
      console.log(lines);
      socket.send(JSON.stringify({ id: "DRAWING", data: lines }));
    }
  };

  // Function to clear the drawing
  const handleClearDrawing = () => {
    setLines([]); // Clear all lines
  };

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
        <p className="app-arm-status">
          ARM Status: <strong>{armStatus}</strong>
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

      <section className="drawing-zone">
        <h2 className="messages-title">Drawing Zone</h2>
        <Stage
          width={536}
          height={300}
          onMouseDown={handleMouseDown}
          onMousemove={handleMouseMove}
          onMouseup={handleMouseUp}
        >
          <Layer>
            {lines.map((line, i) => (
              <Line
                key={i}
                points={line.points}
                stroke="black"
                strokeWidth={2}
              />
            ))}
          </Layer>
        </Stage>
        <button className="clear-button" onClick={handleClearDrawing}>
          Clear Drawing
        </button>
      </section>
    </div>
  );
};

export default App;
