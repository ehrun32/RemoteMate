#include <Servo.h>
#include "HCPCA9685.h"
#define  I2CAdd 0x40
HCPCA9685 HCPCA9685(I2CAdd);

// global variables corresponding to each motor
int Base;
int Shoulder;
int Elbow1;
int Elbow2;
int Wrist;
int ClampAnalog;

int LastShoulder = 206;
int LastElbow1 = 215;
int LastWrist = 205;

//toggle global variable to see if there is feedback
bool DelicateFeedback;

//global variables set to corresponding analog maximum values
int MaxFlex = 510;      
int MaxPulse = 350;
int ClampPulse;
bool reset = true;


void setup() 
{
  Serial.begin(115200);
  //Serial.println("117,200,118,186,145,125,0");

  HCPCA9685.Init(SERVO_MODE);
  HCPCA9685.Sleep(false);
  HCPCA9685.Servo(0, 222);
  HCPCA9685.Servo(1, 206);
  HCPCA9685.Servo(3, 215);
  HCPCA9685.Servo(4, 217);
  HCPCA9685.Servo(6, 205);
  HCPCA9685.Servo(7, 350);
}

void loop() 
{
 if (Serial.available() > 0)
  {
    String data = Serial.readStringUntil('\n');
    char *str = new char[data.length() + 1];
    strcpy(str, data.c_str()); // need the String for other things after converting it to an array of ints, so have to make a copy first
    const size_t bufferSize = 7; 
    int arr[bufferSize];
    char *p = strtok(str, "[,]");
    size_t index = 0;

    //parse the string that is sent to the serial port into an array
    while (p != nullptr && index < bufferSize) 
    {
      arr[index++] = atoi(p); // index each of the parts to convert it to an int
      p = strtok(NULL, ","); //split the char array into the different parts
    }

    //Parse incoming data and assign to global variables
    Base = arr[0];
    Shoulder = arr[1];
    Elbow1 = arr[2];
    Elbow2 = arr[3];
    Wrist = arr[4];
    ClampAnalog = arr[5];
    DelicateFeedback = arr[6];

    Base = constrain(Base, 60, 383);
    Shoulder = constrain(Shoulder, 40, 375);
    Elbow1 = constrain(Elbow1, 50, 395);
    Elbow2 = constrain(Elbow2, 50, 383);
    Wrist = constrain(Wrist, 45, 375);
    


    if (Shoulder > LastShoulder + 100 || Elbow1 > LastElbow1 + 100 || Wrist > LastWrist + 100)
    {
      HCPCA9685.Servo(1, LastShoulder);
      HCPCA9685.Servo(3, LastElbow1);
      HCPCA9685.Servo(6, LastWrist);
    }
    else
    {
      LastShoulder = Shoulder;
      LastElbow1 = Elbow1;
      LastWrist = Wrist;
      HCPCA9685.Servo(1, LastShoulder);
      HCPCA9685.Servo(3, LastElbow1);
      HCPCA9685.Servo(6, LastWrist);
    }

    HCPCA9685.Servo(0, Base);
    HCPCA9685.Servo(4, Elbow2);

    
    delete[] str; //free the memory afterwards
 

    //Serial.println(Base);
    //Serial.println(Shoulder);
    //Serial.println(Elbow1);
    //Serial.println(Elbow2);
    //Serial.println(Wrist);
    //Serial.println(ClampAnalog);
    //Serial.println(DelicateFeedback);

    //Write parsed pwm values to driver


    if (DelicateFeedback == false)
    {
      ClampAnalog = constrain(ClampAnalog, 320, 510);
      ClampPulse = map(ClampAnalog, 320, 510, 56, 350);
      HCPCA9685.Servo(7, ClampPulse);
    }
    else
    {
      int angle = feedback(ClampAnalog);   //calls feedback loop and sends ClampAnalog as an argument
      angle = constrain(angle,56,350); //safety 180 degrees servo motor constraint
      if (ClampAnalog <= MaxFlex)     //deciding logic that writes to the servo motor
      {
        HCPCA9685.Servo(7, angle);
      }
      else
      {
        HCPCA9685.Servo(7, MaxPulse);
      }
    }
  }
  delay(20);  //Figure out delay later, for now I think 20 sec is fine
}

int feedback(int ClampAnalog)          //feedback loop with argument ClampAnalog
{
  int force = analogRead(A1);       //reads current analog FSR force value and prints it
  int angle = map(ClampAnalog,320,MaxFlex,56,MaxPulse);   //analog to digital mapping for variable angle
  if (force >= 200)         //compares force with analog value of desired force
  {
    if (reset == true)
    {
      MaxFlex = ClampAnalog;
      MaxPulse = angle;
      reset = false;          //restricts MaxFlex and MaxPulse from updating if force is maintained or higher
    }
    else
    {
      return angle;
    }
  }
  else              //less than desired force state
  {
    MaxFlex = 510;
    MaxPulse = 350;
    reset = true;
  }
  return angle;
}

 
