import serial
import time

arduino = serial.Serial(port='COM3', baudrate=115200, timeout = 5)

motor1 = 100
motor2 = 200
motor3 = 200
motor4 = 250
motor5 = 125
motor6 = 150

while True: #while serial is open run json string
    try:
        #cmd = f'{{"data" : [{motor1},{motor2},{motor3},{motor4},{motor5},{motor6}]}}'
        cmd= f"{motor1},{motor2},{motor3},{motor4},{motor5},{motor6},1"
        cmd=cmd+'\n'
        print(cmd)
        arduino.write(cmd.encode())
        motor1 = motor1 + 1
        motor2 = motor2 + 1
        motor3 = motor3 + 1
        motor4 = motor4 + 1
        motor5 = motor5 + 1
        motor6 = motor6 + 1
        #for x in range(10):
            #incoming = arduino.readline().decode("utf-8")
            #print(incoming)
        time.sleep(0.05)

    except:
        print("Keyboard Interrupt") # Press Ctrl+C to exit
        break