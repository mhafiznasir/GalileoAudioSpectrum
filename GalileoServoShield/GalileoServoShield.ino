#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  100 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  560 // this is the 'maximum' pulse length count (out of 4096)
uint8_t servonum = 0;
uint16_t pulseLength = 0;
uint16_t pulselength0 = 0;
uint16_t pulselength1 = 0;
uint16_t pulselength2 = 0;
uint16_t pulselength3 = 0;
uint16_t pulselength4 = 0;
uint16_t pulselength5 = 0;
uint16_t pulselength6 = 0;
uint16_t pulselength7 = 0;

uint16_t amplitude[8] = {90, 90, 90, 90, 90, 90, 90, 90};
uint16_t amplitude2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(9600);
  Serial.println("16 channel Servo test!");

  pwm.begin();

  pwm.setPWMFreq(50);  // Analog servos run at ~60 Hz updates
}

int delayTime = 1500;

void loop() {
  // Drive each servo one at a time
// ------------- FOR PRODUCTION -------------
// for (uint8_t i=0; i<=7; i++)
// {
//    pulseLength = map(0, 0, 180, SERVOMIN, SERVOMAX);
//    pwm.setPWM(i, 0, pulseLength);
// } 
//  
// delay(delayTime);


// ------------- FOR TESTING ------------- 
  pulselength0 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(0, 0, pulselength0);
  pulselength1 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(1, 0, pulselength1);
  pulselength2 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(2, 0, pulselength2);
  pulselength3 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(3, 0, pulselength3);
  pulselength4 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(4, 0, pulselength4);
  pulselength5 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(5, 0, pulselength5);
  pulselength6 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(6, 0, pulselength6);
  pulselength7 = map(0, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(7, 0, pulselength7);
  
  delay(delayTime);
  
  int endDegree = 180;
  
  pulselength0 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(0, 0, pulselength0);
  pulselength1 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(1, 0, pulselength1);
  pulselength2 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(2, 0, pulselength2);
  pulselength3 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(3, 0, pulselength3);
  pulselength4 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(4, 0, pulselength4);
  pulselength5 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(5, 0, pulselength5);
  pulselength6 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(6, 0, pulselength6);
  pulselength7 = map(endDegree, 0, 180, SERVOMIN, SERVOMAX);
  pwm.setPWM(7, 0, pulselength7);


  delay(delayTime);
}
