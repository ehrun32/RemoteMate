#include <Servo.h>  // add servo library
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "HCPCA9685.h"
#include <movingAvg.h>

#define  I2CAdd 0x40
#define OUTPUT_READABLE_YAWPITCHROLL
#define INTERRUPT_PIN 2  // use pin 2 on Arduino Uno & most boards
#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)

#define BaseLeft 'a'
#define BaseRight 'd'
#define Elbow2Up 'w'
#define Elbow2Down 's'
#define PrecisionLow 'l'
#define PrecisionMed 'm'
#define PrecisionHigh 'h'
#define Base0 '1'
#define Base45 '2'
#define Base90 '3'
#define Base135 '4'
#define Base180 '5'

HCPCA9685 HCPCA9685(I2CAdd);
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

int i = 0;
int j = 0;
float YawCorrection;
const int FlexPin_Long = A0;
const int FlexPin_Short = A1;

int BasePulse = 222;
int Elbow2Pulse = 217;
int Precision = 10;
int Direction = 0;
int LongFlex;
int ShortFlex;
int AverageFlex;
int Angle;
int MovingAvg;

movingAvg Avg(30);

// ================================================================
// ===               INTERRUPT DETECTION ROUTINE                ===
// ================================================================

volatile bool mpuInterrupt = false;     // indicates whether MPU interrupt pin has gone high
void dmpDataReady() {
    mpuInterrupt = true;
}

void setup() 
{
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
        Wire.setWireTimeout(10000, true); // avoids program crash due to wire.h library
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    Serial.begin(38400);
    while (!Serial);

    // NOTE: 8MHz or slower host processors, like the Teensy @ 3.3V or Arduino
    // Pro Mini running at 3.3V, cannot handle this baud rate reliably due to
    // the baud timing being too misaligned with processor ticks. You must use
    // 38400 or slower in these cases, or use some kind of external separate
    // crystal solution for the UART timer.

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

    HCPCA9685.Init(SERVO_MODE);
    HCPCA9685.Sleep(false);
    HCPCA9685.Servo(0, 222);
    HCPCA9685.Servo(4, 217);
    Avg.begin();
}

// ================================================================
// ===                    MAIN PROGRAM LOOP                     ===
// ================================================================

void loop() {
    // if programming failed, don't try to do anything
    if (!dmpReady) return;
    // read a packet from FIFO
    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
        #ifdef OUTPUT_READABLE_YAWPITCHROLL
            // display Euler angles in degrees
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

            ypr[0] = ypr[0] * 180 / M_PI;
            ypr[1] = ypr[1] * 180 / M_PI;
            ypr[2] = ypr[2] * 180 / M_PI;

            if (j <= 50)
            {
                YawCorrection = ypr[0];
                j++;
                //Serial.println(j);
            }
            
            else
            {
                ypr[0] = ypr[0] - YawCorrection;

                ypr[0] = constrain(ypr[0], -45, 45);
                ypr[1] = constrain(ypr[1], -50, 50);
                ypr[2] = constrain(ypr[2], -30, 30);
                int shoulder = ypr[0];
                int elbow1 = ypr[1];
                int wrist = ypr[2];
                
                shoulder = map(ypr[0], -45, 45, 40, 375);
                shoulder = constrain(shoulder, 40, 375);
                elbow1 = map(ypr[1], -50, 50, 395, 50);
                elbow1 = constrain(elbow1, 50, 395);
                wrist = map(ypr[2], -30, 30, 375, 45);
                wrist = constrain(wrist, 45, 375);

                HCPCA9685.Servo(1, shoulder);
                HCPCA9685.Servo(3, elbow1);
                HCPCA9685.Servo(6, wrist);
                
                Serial.print("ypr\t");
                Serial.print(ypr[0]);
                Serial.print("\t");
                Serial.print(ypr[1]);
                Serial.print("\t");
                Serial.println(ypr[2]);

                //Serial.print("\t");
                //Serial.println(shoulder);
                //Serial.print("\t");
                //Serial.print(elbow1);
                //Serial.print("\t");
                //Serial.println(wrist);

                //keyboard();
                clamp();
            }

        #endif

        // blink LED to indicate activity
        blinkState = !blinkState;
        digitalWrite(LED_PIN, blinkState);
    }
    //delay(20);
}

void keyboard()
{
  char buffer = 0;
  Serial.readBytes(&buffer, 1); //Serial.readbytes(buffer, length)

  if (buffer == BaseLeft)
  {
    BasePulse -= Precision;
    BasePulse = constrain(BasePulse, 60, 383);
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == BaseRight)
  {
    BasePulse += Precision;
    BasePulse = constrain(BasePulse, 60, 383);
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == Elbow2Down)
  {
    Elbow2Pulse -= Precision;
    Elbow2Pulse = constrain(Elbow2Pulse, 50, 383);
    HCPCA9685.Servo(4, Elbow2Pulse);
    buffer = 0;
  }

  if (buffer == Elbow2Up)
  {
    Elbow2Pulse += Precision;
    Elbow2Pulse = constrain(Elbow2Pulse, 50, 383);
    HCPCA9685.Servo(4, Elbow2Pulse);
    buffer = 0;
  }

  //precision settings
  if (buffer == PrecisionLow)
  {
    Precision = 10;
    buffer = 0;
  }

  if (buffer == PrecisionMed)
  {
    Precision = 5;
    buffer = 0;
  }

  if (buffer == PrecisionHigh)
  {
    Precision = 1;
    buffer = 0;
  }

  //quick base positioning with precision enabled
  if (buffer == Base0)
  {
    Direction = BasePulse - 60;
    i = (abs(Direction / Precision));
    for(i; i > 0; i--)
    {
      BasePulse = BasePulse - Precision * (Direction / abs(Direction));
      BasePulse = constrain(BasePulse, 60, 383);
      HCPCA9685.Servo(0, BasePulse);
      delay(50);
    }
    BasePulse = 60;
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == Base45)
  {
    Direction = BasePulse - 141;
    i = (abs(Direction / Precision));
    for(; i > 0; i--)
    {
      BasePulse = BasePulse - Precision * (Direction / abs(Direction));
      BasePulse = constrain(BasePulse, 60, 383);
      HCPCA9685.Servo(0, BasePulse);
      delay(50);
    }
    BasePulse = 141;
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == Base90)
  {
    Direction = BasePulse - 222;
    i = (abs(Direction / Precision));
    for(i; i > 0; i--)
    {
      BasePulse = BasePulse - Precision * (Direction / abs(Direction));
      BasePulse = constrain(BasePulse, 60, 383);
      HCPCA9685.Servo(0, BasePulse);
      delay(50);
    }
    BasePulse = 222;
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == Base135)
  {
    Direction = BasePulse - 302;
    i = (abs(Direction / Precision));
    for(i; i > 0; i--)
    {
      BasePulse = BasePulse - Precision * (Direction / abs(Direction));
      BasePulse = constrain(BasePulse, 60, 383);
      HCPCA9685.Servo(0, BasePulse);
      delay(50);
    }
    BasePulse = 302;
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  if (buffer == Base180)
  {
    Direction = BasePulse - 383;
    i = (abs(Direction / Precision));
    for(i; i > 0; i--)
    {
      BasePulse = BasePulse - Precision * (Direction / abs(Direction));
      BasePulse = constrain(BasePulse, 60, 383);
      HCPCA9685.Servo(0, BasePulse);
      delay(50);
    }
    BasePulse = 383;
    HCPCA9685.Servo(0, BasePulse);
    buffer = 0;
  }

  Serial.setTimeout(20);
}

void clamp()
{
  LongFlex = analogRead(FlexPin_Long);
  LongFlex = constrain(LongFlex, 200, 460);
  ShortFlex = analogRead(FlexPin_Short);
  ShortFlex = constrain(ShortFlex, 360,620);

  AverageFlex = (LongFlex + ShortFlex)/2; //min 280 and max 540
  MovingAvg = Avg.reading(AverageFlex);
  MovingAvg = constrain(MovingAvg, 280, 540);
  Angle = map(MovingAvg, 280, 540, 56, 350);
  HCPCA9685.Servo(7, Angle);
  //delay(5);
}
