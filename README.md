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

### 


## Next steps

Things to improve:



## References



