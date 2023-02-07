const Websocket = require("ws");

const wss = new Websocket.Server({ port: 8000, host: "liam-linux" });

//initial parser.on call to make sure we are reading the serial port right away
//this is to ensure that no message sent from the websocket is incomplete at the start

wss.on("connection", (ws) => {
  console.log("A new client is connected");
  ws.send("Hi Arun");
  // const user = { name: "Liam", age: 22, country: "Canada" };
});
