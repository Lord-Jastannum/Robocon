/*************************************************** 
  This is an example for our Adafruit 16-channel PWM & Servo driver
  GPIO test - this will set a pin high/low

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/products/815

  These drivers use I2C to communicate, 2 pins are required to  
  interface.

  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// called this way, it uses the default address 0x40
// Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
// you can also call it with a different address you want
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x41);
// you can also call it with a different address and I2C interface
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40, Wire);
int feed_pwm=10;
int feed_dir=24;
void setup() {
  Serial.begin(9600);
  Serial.println("GPIO test!");
analogWrite(13,255);
  pwm.begin();
  pwm.setPWMFreq(1000);  // Set to whatever you like, we don't use it in this demo!
  pinMode(11,OUTPUT);
    pinMode(feed_dir,OUTPUT);

        digitalWrite(11,HIGH);
      analogWrite(12,0);
            analogWrite(feed_pwm,0);
  // if you want to really speed stuff up, you can go into 'fast 400khz I2C' mode
  // some i2c devices dont like this so much so if you're sharing the bus, watch
  // out for this!
  // Wire.setClock(400000);
}
int input=0;
void loop() {
  // Drive each pin in a 'wave'
  // for (uint8_t pin=0; pin<16; pin++) {
    if(Serial.available())
    {
      String inp=Serial.readString();
      input=inp.toInt();
    }     // turns pin fully off
  // }
    if(input==1){
    pwm.setPWM(13, 0, 4096); 
    Serial.println("HIGH");      // turns pin fully on
    delay(5000);
    pwm.setPWM(13, 0, 0);
    Serial.println("LOW");  
    delay(5000);
    Serial.println("Input 1");
    }
    else if(input==2)
    {
    pwm.setPWM(12, 0, 4096); 
    Serial.println("HIGH");      // turns pin fully on
    delay(5000);
    pwm.setPWM(12, 0, 0); 
    Serial.println("LOW");      
    delay(5000);
    Serial.println("Input 2");
    }

    if(input==3)
    {
      digitalWrite(11,HIGH);
      analogWrite(12,255);
      delay(400);
      digitalWrite(11,LOW);
      analogWrite(12,255);
      delay(1000);
      digitalWrite(11,LOW);
      analogWrite(12,0);  
      delay(2000);  }
    if(input==4)
    {
      digitalWrite(11,HIGH);
      analogWrite(12,0);
    }
    if(input==5)
    {
      digitalWrite(11,HIGH);
      analogWrite(12,255);
      delay(200);
      digitalWrite(11,LOW);
      analogWrite(12,255);
      delay(500);
      analogWrite(12,0);
      digitalWrite(feed_dir,HIGH);
      analogWrite(feed_pwm,255);
      input=0;

    }
 if(input==6)
  {
    digitalWrite(feed_dir,HIGH);
      analogWrite(feed_pwm,0);
      input=0; }


 }
