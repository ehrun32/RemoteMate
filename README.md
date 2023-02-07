# RemoteMate 🦾



## Project Summary
This project propose to design and build a fully remotely controllable robotic arm which perfectly mimics
human arm movement, can pick up and clamp onto objects, and has tactile feedback based on
pressure asserted to objects. The movement of the robotic arm will be controlled via a wearable
glove that the operator wears, and there will be a central web application which will control the robotic 
arm’s features which include clamp pressure options, movement controls for extra axes of freedom, and
movement sensitivity options.

## Methodology
![image](https://user-images.githubusercontent.com/57046416/217128001-85da4ccc-5645-4d18-becb-7c64d497bc03.png)

The following diagram above shows a general block diagram overview of the entire system. This
serves to show the end-to-end control of the robotic arm and serves to show the emulation
process for the problem at hand (the surgical process). Highlighted in blue is the glove control
which consists of an MPU6050, two flex sensors and Laptop A which produces a total of 4
values that are to control 4 motors located on the robotic arm. Laptop A serves as a general
transmitter for the data sent as well as a general control station. The wireless data transfer
highlighted by the cyan colour serves as the wireless channel which includes a server, WLAN
network produced by some router or mobile hotspot and Laptop B which serves as a receiver of
the data sent. The web application is initiated by the server and can be seen by the control user
located at Laptop A and provides WASD control for the remaining 2 motors as well as other
implemented toggle features. The robotic arm block highlighted in red is the receiving control
station which handles the entirety of the robotic arm movement. Finally, the feedback block
highlighted in green serves in handling the video stream sent back to the control user and
emulates surgical vision. It includes a 2D dimensional identification algorithm with a use of a
reference object.

## Features
### Movement
- MPU6050
  - Yaw, pitch and roll values are used properly to control 3 joints on the
  robotic arm including shoulder, eblow2 and wrist. Movement is smooth
  and fairly accurate considering the hardware used.
- Flex Sensors
  - Opening and closing of the fist controls the clamp
### Safety Implementations
- Gimbal Lock Check
  - Robotic arm control successfully locks when the user hand is out of
  bounds. Implementation of the x-y-z hierarchy is successful
- Moving Average Filter
  - Moving average filter implementation on the sampled flex sensor values is
  successful. The clamp control is less jittery and user has smoother control
- Spike filter
  - Filter function on the receiving side successfully discards values when
  data sent is present with too high of an increment (beyond threshold
  value). Robust control of the arm is maintained.
### Web Movement
-WASD
  - WASD control successfully controls the remaining 2 joints on the robotic arm
  including the base and elbow1.
### Toggle Features
- Precision
  - Precision includes 3 settings namely “Low”, “Medium” and “High”.
  These settings successfully change the precision increments of the WASD
  control for the base and elbow1 control.
- Clamp Feedback Toggle
  - Feedback toggle is successfully sent to the robotic arm. This enables the
  clamp to take into consideration the FSR sensor force values and make
  comparisons to the user input coming from the flex sensors. Clamp
  successfully stops applying further motor torque/pressure when threshold
  is surpassed.
### Video Stream
- Web application can successfully retrieve the video stream hosted by the camera
on the robotic arm receiving side. This is seen through a hosted Youtube stream.
### Wireless connection
- Successfully handles incoming and outgoing data through web socket connection.
Latency is dependent on the signal strength and possible interference or network
congestion.

## Prototype
The following section discusses the performance of the integrated prototype as a whole. The
prototype is completely functional and all promised features have been delivered. The prototype
contains 3 major components including the input/output control of the robotic arm, the web
application and the user feedback with dimension algorithm.

### Glove and Robotic Arm Control
The final prototype for the globe and robotic arm modules are shown below. The glove serves as
the input control unit and the robotic arm serves as the output control unit.
![image](https://user-images.githubusercontent.com/57046416/217128702-a53b75df-6068-48e9-a1db-198fb295625b.png)
![image](https://user-images.githubusercontent.com/57046416/217128735-c08c5e6e-e092-41d5-9979-11c39e60e699.png)

### Web Application/Server
The final web application design along with its implemented features are shown below.

![image](https://user-images.githubusercontent.com/57046416/217128835-e37ca4ae-0c75-4a78-95d9-a2ec1572049a.png)


## Next steps

Things to improve:



## References



