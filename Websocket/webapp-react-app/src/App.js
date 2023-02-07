import React from "react";
import YoutubeComponent from "./YoutubeComponent";

import "./App.css";

export default function App() {
  return (
    <div className="App">
      <header className="App-header">
        <h1>Robotic Arm Web App</h1>
        <YoutubeComponent />
        <a
          className="App-link"
          href="https://www.npmjs.com/package/ws"
          target="_blank"
          rel="noopener noreferrer"
        >
          Websocket Link
        </a>
      </header>
      <script src="server.js"></script>
    </div>
  );
}
