#include <Servo.h>  // add servo library
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
#endif
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
//#include "HCPCA9685.h"
#include <movingAvg.h>

#define  I2CAdd 0x40
#define OUTPUT_READABLE_YAWPITCHROLL
#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno
#define LED_PIN 13 // use pin 13 on Arduino Uno

//HCPCA9685 HCPCA9685(I2CAdd);
MPU6050 mpu;

bool blinkState = false;
// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

int j = 0;
float YawCorrection;
float PitchCorrection;
float RollCorrection;
const int FlexPin_Long = A1;
const int FlexPin_Short = A0;

bool LockShoulderWrist = false;
bool LockElbow = false;
int LastShoulderLock = 0;
int LastElbowLock = 0;
int LastWristLock = 0;
bool reset = true;

int LongFlex;
int ShortFlex;
int AverageFlex;
int Angle;
int MovingAvg;

movingAvg Avg(5); //change if clamp is opening/closing too slow/fast

// ================================================================
// ===                         SETUP                            ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
  mpuInterrupt = true;
}

void setup() 
{
  // join I2C bus
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
    Wire.setWireTimeout(5000, true); // avoids program crash due to wire.h library
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  Serial.begin(115200); // initialize maximum baudrate
  while (!Serial);

  // initialize device
  Serial.println(F("Initializing I2C devices..."));
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  // verify connection
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));

  // wait for ready
  //Serial.println(F("\nSend any character to begin DMP programming and demo: "));
  //while (Serial.available() && Serial.read()); // empty buffer
  //while (!Serial.available());                 // wait for data
  //while (Serial.available() && Serial.read()); // empty buffer again

  // load and configure the DMP
  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  mpu.setXGyroOffset(220);
  mpu.setYGyroOffset(76);
  mpu.setZGyroOffset(-85);
  mpu.setZAccelOffset(1788); // 1688 factory default for my test chip

  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // turn on the DMP, now that it's ready
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    // enable Arduino interrupt detection
    Serial.print(F("Enabling interrupt detection (Arduino external interrupt "));
    Serial.print(digitalPinToInterrupt(INTERRUPT_PIN));
    Serial.println(F(")..."));
    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();

    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    Serial.println(F("DMP ready! Waiting for first interrupt..."));
    dmpReady = true;

    // get expected DMP packet size for later comparison
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  // configure LED for output
  pinMode(LED_PIN, OUTPUT);

  //HCPCA9685.Init(SERVO_MODE);
  //HCPCA9685.Sleep(false);
  //HCPCA9685.Servo(0, 222);
  //HCPCA9685.Servo(4, 217);
  Avg.begin();
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
// exit if mpu/dmp programming failed
  if (!dmpReady) return;
  // read packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
    #ifdef OUTPUT_READABLE_YAWPITCHROLL
      // acquire dmp values
      mpu.dmpGetQuaternion(&q, fifoBuffer);
      mpu.dmpGetGravity(&gravity, &q);
      mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

      // calculate ypr values in degrees
      ypr[0] = ypr[0] * 180 / M_PI;
      ypr[1] = ypr[1] * 180 / M_PI;
      ypr[2] = ypr[2] * 180 / M_PI;

      // additional calibration while MPU stabilizes
      if (j <= 15) //change back to 1.5k
      {
        YawCorrection = ypr[0];
        PitchCorrection = ypr[1];
        RollCorrection = ypr[2];
        j++;
        //Serial.println(j);
      }
      
      else
      {
        ypr[0] = ypr[0] - YawCorrection;
        ypr[1] = ypr[1] - PitchCorrection;
        ypr[2] = ypr[2] - RollCorrection;

        // assign ypr values to corresponding component
        int shoulder = ypr[0];
        int elbow1 = ypr[1];
        int wrist = ypr[2];

        // check for gimbal lock
        GimbalLockCheck(shoulder, elbow1, wrist);

        if (LockShoulderWrist == true)
        {
          elbow1 = map(elbow1, -50, 50, 395, 50);
          elbow1 = constrain(elbow1, 50, 395);
          //HCPCA9685.Servo(1, LastShoulderLock);
          //HCPCA9685.Servo(3, elbow1);
          //HCPCA9685.Servo(6, LastWristLock);

          Serial.print("[0," + String(LastShoulderLock) + "," + String(elbow1) + ",0," + String(LastWristLock) + ",");
        }
        else if (LockElbow == true)
        {
          shoulder = map(shoulder, -45, 45, 40, 375);
          shoulder = constrain(shoulder, 40, 375);
          wrist = map(wrist, -30, 30, 375, 45);
          wrist = constrain(wrist, 45, 375);
          //HCPCA9685.Servo(1, shoulder);
          //HCPCA9685.Servo(3, LastElbowLock);
          //HCPCA9685.Servo(6, wrist);

          Serial.print("[0," + String(shoulder) + "," + String(LastElbowLock) + ",0," + String(wrist) + ",");
        }
        else
        {
          shoulder = map(shoulder, -45, 45, 40, 375);
          shoulder = constrain(shoulder, 40, 375);
          elbow1 = map(elbow1, -50, 50, 395, 50);
          elbow1 = constrain(elbow1, 50, 395);
          wrist = map(wrist, -30, 30, 375, 45);
          wrist = constrain(wrist, 45, 375);
          //HCPCA9685.Servo(1, shoulder);
          //HCPCA9685.Servo(3, elbow1);
          //HCPCA9685.Servo(6, wrist);

          Serial.print("[0," + String(shoulder) + "," + String(elbow1) + ",0," + String(wrist) + ",");
        }

        clamp();

        Serial.println("");
        //Serial.println("state:" + String(LockShoulderWrist) + "," + String(LockElbow) + "," + String(reset));
      }

    #endif

    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
  }
  delay(50);
}

int GimbalLockCheck(int shoulder, int elbow, int wrist)
{
  if ((elbow < -50 || elbow > 50) && LockElbow == false)
  {
    LockShoulderWrist = true;
    if (reset == true)
    {
      LastShoulderLock = shoulder;
      LastWristLock = wrist;
      LastShoulderLock = map(LastShoulderLock, -45, 45, 40, 375);
      LastShoulderLock = constrain(LastShoulderLock, 40, 375);
      LastWristLock = map(LastWristLock, -30, 30, 375, 45);
      LastWristLock = constrain(LastWristLock, 45, 375);
      reset = false;
    }
    return;
  }
  else
  {
    LockShoulderWrist = false;
  }

  if ((wrist < -30 || wrist > 30) && LockShoulderWrist == false)
  {
    LockElbow = true;
    if (reset == true)
    {
      LastElbowLock = elbow;
      LastElbowLock = map(LastElbowLock, -50, 50, 395, 50);
      LastElbowLock = constrain(LastElbowLock, 50, 395);
      reset = false;
    }
    return;
  }
  else
  {
    LockElbow = false;
  }

  reset = true;
  return;
}

void clamp()
{
  LongFlex = analogRead(FlexPin_Long);
  LongFlex = constrain(LongFlex, 200, 460);
  ShortFlex = analogRead(FlexPin_Short);
  ShortFlex = constrain(ShortFlex, 360,620);

  AverageFlex = (LongFlex + ShortFlex)/2; // min 280 and max 540
  MovingAvg = Avg.reading(AverageFlex);
  MovingAvg = constrain(MovingAvg, 320, 510);
  Serial.print(String(MovingAvg) + ",0]");
  //Angle = map(MovingAvg, 280, 540, 56, 350);
  //HCPCA9685.Servo(7, Angle);
  //delay(5);
}
