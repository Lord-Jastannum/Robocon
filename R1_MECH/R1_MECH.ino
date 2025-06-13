#include <Encoder.h>
#include <VescUart.h>
#include <Servo.h>
#include <Pixy2I2C.h>

Pixy2I2C pixy;
VescUart UART;

int buttons = 0;

bool limitSwitchState[8] = {false};           // 1        3   5
int limitSwitch[6]={14,15,41,40,38,39};       // 0        2   4
int flag_rec_r=0;
int flag_rec_l=0;
int flag_feed=0;

//driblling 
int roller_dir=11;
int roller_pwm=12;
int flag_roller = 0;

Servo servo_left;
Servo servo_right;
Servo servo_stopper;
Servo servo_cam;

int piston[3][2] = {{26,27} , {31,30} , {28,29}};
/////////dribbling


// feeding
volatile int feeder_pwm=19;
volatile int feeder_dir=16;

///////// feeding


// recieving 
int rcv_pwm[2] = {23, 22};
int rcv_dir[2] = {21, 20};

///////// recieving 


void setup() {

  Serial.begin(200000);
  Serial2.begin(115200);             // teensy - teensy uart
  Serial7.begin(115200);             // bldc - teensy uart
  UART.setSerialPort(&Serial7);

  // dribbling
  pixy.init();

  // teensy led
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);

  servo_left.attach(2);
  servo_right.attach(1); 
  servo_stopper.attach(3);
  servo_cam.attach(0);

  servo_left.write(15);
  servo_right.write(180-15);
  servo_stopper.write(0);
  servo_cam.write(0);

  pinMode(roller_dir, OUTPUT);
  pinMode(roller_pwm, OUTPUT);

  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 2; j++){
      pinMode(piston[i][j], OUTPUT);
    }
  }

  pistonControl(0, false);
  pistonControl(1, false);
  pistonControl(2, false);

  // feeding
  pinMode(feeder_pwm, OUTPUT);
  pinMode(feeder_dir, OUTPUT);

  analogWrite(feeder_pwm, 0);
  digitalWrite(feeder_dir, LOW);


  // recieving
  for (int i = 0; i < 2; i++) {
    pinMode(rcv_pwm[i], OUTPUT);
    pinMode(rcv_dir[i], OUTPUT);
  }

  // Initialize motor to stop
  for (int i = 0; i < 2; i++) {
    analogWrite(rcv_pwm[i], 0);
    digitalWrite(rcv_dir[i], LOW);
  }


  // Initializing limit switches 
  for(int i=0;i<6;i++)
  {
    pinMode(limitSwitch[i],INPUT_PULLUP);
  }
  
  // Initializing flag values for feeding and recieving 
  flag_feed = digitalRead(limitSwitch[0]);
  flag_rec_r = digitalRead(limitSwitch[2]);
  flag_rec_l = digitalRead(limitSwitch[4]);


  analogWriteResolution(14);
  analogWriteFrequency(0, 9000);
}


int constraint(int value, int min_val, int max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}


//DRIBBLING
void servomotion(int start_angle, int end_angle){
  if(start_angle<end_angle){ 
    for(int angle=start_angle;angle<=end_angle;angle++){
      servo_right.write(180-angle);
      servo_left.write(angle-3);
      delay(15);
    }     
  }
  else if(start_angle>end_angle){
    for(int angle=start_angle;angle>=end_angle;angle--){
      servo_right.write(180-angle);
      servo_left.write(angle-3);
      delay(15);
    }
  }

}

void rollers(){
  if (flag_roller == 0){
    digitalWrite(roller_dir,HIGH);
    analogWrite(roller_pwm,255*64);
    flag_roller = 1;
  }
  else if (flag_roller == 1){
    digitalWrite(roller_dir,LOW);
    analogWrite(roller_pwm,0);
    flag_roller = 0;
  }
}

// void rollers(){
//   digitalWrite(roller_dir,HIGH);
//   analogWrite(roller_pwm,255*64);

// }

// void stoprollers(){
//   digitalWrite(roller_dir,LOW);
//   analogWrite(roller_pwm,0);

// }

void pistonControl(int pistonNum, bool extend){
  digitalWrite(piston[pistonNum][0], extend);
  digitalWrite(piston[pistonNum][1], !extend);
  delay(500);
  digitalWrite(piston[pistonNum][0], LOW);
  digitalWrite(piston[pistonNum][1], LOW);

}

// pistonControl(0, true);   // ---> extend
// pistonControl(0, false);   // ---> retract
/////DRIBBLING


void loop() {

  for (int i = 0; i < 6; i++){
    limitSwitchState[i] = digitalRead(limitSwitch[i]);
  }

  if (Serial2.available() >=  sizeof(buttons)) {
    // Read buttons
    Serial2.readBytes((char*)&buttons, sizeof(buttons));
    Serial.print("Buttons: ");
    Serial.println(buttons);
  }

  // FEEDING
  if (buttons == 2 && flag_feed == 0) {
    buttons = 0;
    Serial.println("Feed UP");
    while (limitSwitchState[1] != LOW){
      digitalWrite(feeder_dir, LOW);
      analogWrite(feeder_pwm, 255*64);
      limitSwitchState[1] = digitalRead(limitSwitch[1]);
    }
    digitalWrite(feeder_dir, LOW);
    analogWrite(feeder_pwm, 255*0);
    flag_feed = 1;
  }
  else if(buttons == 2 && flag_feed == 1){
    buttons = 0;
    Serial.println("Feed DOWN");
    while (limitSwitchState[0] != LOW){
      digitalWrite(feeder_dir, HIGH);
      analogWrite(feeder_pwm, 255*64);
      limitSwitchState[0] = digitalRead(limitSwitch[0]);
    }
    digitalWrite(feeder_dir, LOW);
    analogWrite(feeder_pwm, 255*0);
    flag_feed = 0;
  }

  // RECIEVING 
  else if (buttons == 8 && flag_rec_r == 0 && flag_rec_l == 0) {
    buttons = 0;
    Serial.println("REC UP");
    while (limitSwitchState[3] != LOW || limitSwitchState[5] != LOW){

      if(limitSwitchState[3] != LOW)
      {
        digitalWrite(rcv_dir[0], HIGH);
        analogWrite(rcv_pwm[0], 255*64);
      }
      else
      {
        digitalWrite(rcv_dir[0], HIGH);
        analogWrite(rcv_pwm[0], 255*0);
        flag_rec_r = 1;
      }
      if(limitSwitchState[5] != LOW)
      {
        digitalWrite(rcv_dir[1], HIGH);
        analogWrite(rcv_pwm[1], 255*64);
      }
      else
      {
        digitalWrite(rcv_dir[1], HIGH);
        analogWrite(rcv_pwm[1], 255*0);
        flag_rec_l = 1;
      }
      limitSwitchState[3] = digitalRead(limitSwitch[3]);
      limitSwitchState[5] = digitalRead(limitSwitch[5]);
    }
    limitSwitchState[3] = digitalRead(limitSwitch[3]);
    limitSwitchState[5] = digitalRead(limitSwitch[5]);

    digitalWrite(rcv_dir[0], HIGH);
    analogWrite(rcv_pwm[0], 255*0);
    digitalWrite(rcv_dir[1], HIGH);
    analogWrite(rcv_pwm[1], 255*0);
  }
  else if(buttons == 8 && flag_rec_r == 1 && flag_rec_l == 1){
    Serial.println("REC DOWN");
    while (limitSwitchState[2] != LOW || limitSwitchState[4] != LOW){
      if(limitSwitchState[2] != LOW)
      {
        digitalWrite(rcv_dir[0], LOW);
        analogWrite(rcv_pwm[0], 255*64);
      }
      else
      {
        digitalWrite(rcv_dir[0], HIGH);
        analogWrite(rcv_pwm[0], 255*0);
        flag_rec_r=0;
      }
      if(limitSwitchState[4] != LOW)
      {
        digitalWrite(rcv_dir[1], LOW);
        analogWrite(rcv_pwm[1], 255*64);
      }
      else
      {
        digitalWrite(rcv_dir[1], HIGH);
        analogWrite(rcv_pwm[1], 255*0);
        flag_rec_l=0;
      }
      limitSwitchState[2] = digitalRead(limitSwitch[2]);
      limitSwitchState[4] = digitalRead(limitSwitch[4]);
    }
    limitSwitchState[2] = digitalRead(limitSwitch[2]);
    limitSwitchState[4] = digitalRead(limitSwitch[4]);
    
    digitalWrite(rcv_dir[1], HIGH);
    analogWrite(rcv_pwm[1], 255*0);
    digitalWrite(rcv_dir[0], HIGH);
    analogWrite(rcv_pwm[0], 255*0);
  }

  // DRIBBLING
  else if (buttons == 4) {
    buttons = 0;
    Serial.println("DRIBBLING");

    servo_stopper.write(90); 
    pistonControl(1, true);
    servomotion(15,90);
    delay(1000);

    pistonControl(0, true);
    delay(100);
    pistonControl(0, false);
    delay(50);

    rollers();
    delay(600);

    pixy.setLamp(1, 1);
    while (true) {
      pixy.ccc.getBlocks();
      if (pixy.ccc.numBlocks) {
      pistonControl(2, true);
      delay(600);
      pistonControl(2, false);
        Serial.println("Detected");
        pixy.setLamp(0, 0);
        break;
      }
    }
    delay(200);
    servomotion(90,15);
    rollers();
    delay(500);
    pistonControl(1, false);

    servo_stopper.write(0);  
  }
}