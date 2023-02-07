const Websocket = require("ws");

const wss = new Websocket.Server({ port: 8000, host: "172.20.10.2" }); //change this to your sending IP

const { SerialPort } = require("serialport");
const { ReadlineParser } = require("@serialport/parser-readline");

const parser = new ReadlineParser({ delimiter: "\r\n" });

const port = new SerialPort({ path: "/dev/ttyACM0", baudRate: 115200 }); //change this to your com port

port.pipe(parser);

//right or left
BasePulse = Math.min(Math.max(parseInt(222), 60), 383);

//up or down
Elbow2Pulse = Math.min(Math.max(parseInt(217), 60), 383);
toggle = 0;
precision = 10;

//initial parser.on call to make sure we are reading the serial port right away
//this is to ensure that no message sent from the websocket is incomplete at the start
parser.on("data", function (data) {});

wss.on("connection", (ws) => {
  console.log("A new client is connected");
  setTimeout(() => {
    //sending messages from the comm port
    parser.on("data", function (data) {
      //depack and repack with the wasd and toggle stuff
      const dataArray = JSON.parse(data);

      dataArray[0] = BasePulse;
      dataArray[3] = Elbow2Pulse;
      dataArray[6] = toggle;
      console.log(dataArray);
      ws.send(JSON.stringify(dataArray));
    });
  }, 10000);

  // const user = { name: "Liam", age: 22, country: "Canada" };

  //recieve messages from client
  ws.on("message", (data) => {
    const webAppMessage = data.toString();
    switch (webAppMessage) {
      case "r":
        BasePulse += precision;
        if (BasePulse >= 383) {
          BasePulse = 383;
        }
        break;
      case "l":
        BasePulse -= precision;
        if (BasePulse <= 222) {
          BasePulse = 222;
        }
        break;
      case "u":
        Elbow2Pulse += precision;
        if (Elbow2Pulse >= 383) {
          Elbow2Pulse = 383;
        }
        break;
      case "d":
        Elbow2Pulse -= precision;
        if (Elbow2Pulse <= 217) {
          Elbow2Pulse = 217;
        }
        break;
      case "t-on":
        toggle = 1;
        break;
      case "t-off":
        toggle = 0;
        break;
      case "highP":
        precision = 1;
        break;
      case "medP":
        precision = 5;
        break;
      case "lowP":
        precision = 10;
        break;
      default:
    }
  });
});
