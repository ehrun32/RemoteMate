//instantiating the client websocket connection to the server liam-linux
var WebSocket = require("ws");
var SerialPort = require("serialport");

const websocket = new WebSocket("ws://172.20.10.2:8000");

//instantiating the parser for the serial port

const parsers = SerialPort.parsers;
const parser = new parsers.Readline({
  delimiter: "\r\n",
});

var port = new SerialPort("COM5", {
  baudRate: 115200,
  dataBits: 8,
  parity: "none",
  stopBits: 1,
  flowControl: false,
}).setEncoding("utf-8");

port.pipe(parser);

//adding a listener to notify when the client script is connected to the websocket server
websocket.addEventListener("open", () => {
  console.log("We are connected");
});

//when we recieve a message from the server through the websocket, we write this to the port to be used in arduino script
websocket.addEventListener("message", (e) => {
  port.write(e.data + "\n");
  console.log(e.data);
});
