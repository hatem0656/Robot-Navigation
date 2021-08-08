#include <SoftwareSerial.h>
#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;
Vector norm;
// Timers
unsigned long timer = 0;
float timeStep = 0.01;
float yaw = 0;

#define RX 10
#define TX 11
SoftwareSerial Bt(RX, TX);

#define Dir_Right_M  8
#define PWM_Right_M  5
#define Dir_Left_M  6
#define PWM_Left_M  7
int velocity = 200;
#define rot_velocity 250

#define RDetector 9
#define LDetector 13
#define Buzzerr 12

/*
   Navigation code for team The Shield wriiten by Hatem and Mostafa . Hope it will work well ^_^
*/

bool connection = false;
bool newMine = false;
bool justTurned = true;
byte  Direction , i   ;
float prev_distX , prev_distY , distX = 50, distY , linear_pulses ,  linear_pulsesYold , linear_pulsesXold ;
int sent_distY;
volatile unsigned long left_pulses, right_pulses   ; // pulses in forward and backward directions

typedef enum { Interrupt_zero , Interrupt_one ,  Interrupt0_pin , Interrupt1_pin   }; // Interrupt_zero on pin 2 , Interrupt_one on pin 3 .
const byte prox = 4;
byte count , esc , mine  = 0;
unsigned long now = 0;

unsigned long Htimer = 0;
bool prev_detectorReading = LOW;

byte place = 0;
int data[6] = {} ;


void Right_encoder () {
  left_pulses ++;
}

void Left_encoder () { // left encoder willl control
  right_pulses ++;
}

//37.68cm     1rot      2000pulses
//100cm       2.65rot   5300pulses

void setup() {
  Bt.begin(9600);
  Serial1.begin(9600);//Data transmission
  Serial.begin(115200);

  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }
  mpu.calibrateGyro();
  mpu.setThreshold(3);


  //--------------ENCODER--------------//
  pinMode(Interrupt0_pin , INPUT_PULLUP);
  pinMode(Interrupt1_pin , INPUT_PULLUP);
  attachInterrupt( Interrupt_zero , Right_encoder , RISING);
  //attachInterrupt( Interrupt_one , Left_encoder , RISING);
  //i ++ ;

  //--------------DRIVER--------------//
  pinMode(Dir_Right_M , OUTPUT);
  pinMode(PWM_Right_M , OUTPUT);
  pinMode(Dir_Left_M, OUTPUT);
  pinMode(PWM_Left_M , OUTPUT);

  //--------------Detector&&Buzzerr--------------//
  pinMode(RDetector , INPUT);
  pinMode(LDetector , INPUT);
  pinMode(prox , INPUT_PULLUP);
  pinMode(Buzzerr , OUTPUT);
  digitalWrite(Buzzerr, HIGH);
  digitalWrite(Dir_Right_M , 1);
  digitalWrite( Dir_Left_M , 1);
  analogWrite( PWM_Left_M , 0);
  analogWrite( PWM_Right_M , 0);

  //  conc();
}

void loop() {
  data[0] = '#';
  data[1] = int(distX) % 256;
  data[2] = distX / 256;
  data[3] = int(distY) % 256;
  data[4] = distY / 256;
  data[5] = place;
  linear_pulses = (left_pulses + right_pulses)  ;
  timer = millis();
  norm = mpu.readNormalizeGyro();
  yaw = yaw + norm.ZAxis * timeStep;

  //  digitalWrite(Buzzerr , 0);
   //   Serial.print(left_pulses);
    //  Serial.print("   ");
     // Serial.print(right_pulses);
     // Serial.print("   ");
  //  Serial.print(distY);
  //  Serial.print("   ");
  //  Serial.print(distX  );
  //  Serial.print("   ");
  //Serial.print(Direction  );
  // Serial.print(digitalRead(RDetector));
  //Serial.println();

  if (  detectorNoise() == false) {


    digitalWrite( Buzzerr , LOW);
    if ( esc == 0 ) {

      if (digitalRead(prox) == LOW ) {
        count = 0 ;
        place = 1 ; //above ground
        esc = 1 ;
        mine = 1 ;

      } else if ( (millis() - now) > 2000) {

        count = 0 ;
        place = 0 ; //under ground
        esc = 1 ;
        mine = 1 ;
      }
    }
  }
  if ( mine == 1) {
    if ( abs(prev_distX - distX) >= 50  || abs(prev_distY - distY) >= 50 ) {
//     Serial.println((distX / 100.0));
//      Serial.println((distY / 100.0));
//      Serial.println(place);
      //        Serial.println(yaw);
      //        Serial.println("===============");
      i = 0;
      newMine = true;
      Serial1.write(data[0]);
      prev_distX = distX ;
      prev_distY = distY ;
      esc = 0 ;
      mine = 0 ;
    }
  }
   
  if (Serial1.available() && newMine == true) {
    if (Serial1.read() == '$') {
      Serial1.write(data[i]);
    } else {
      i++;
      if (i == 5) {
        
        Serial1.write(data[i]);
        //Serial.print(data[i]);  Serial.print("  ");
        //Serial.println("\nNew mine    ");
        newMine = false;
        i = 0;
  //      Serial.println();
      } else {
        Serial1.write(data[i]);
        // Serial.print(data[i]);  Serial.print("  ");
      }
    }
  }
  //--------------Motion Bt--------------//
  if (Bt.available()) {
    char recv = Bt.read();
    //Serial.println(recv);
    switch (recv) {
      case '1':
        velocity = 80;
        break;
      case '2':
        velocity = 100;
        break;
      case '3':
        velocity = 120;
        break;
      case '4':
        velocity = 140;
        break;
      case '5':
        velocity = 160;
        break;
      case '6':
        velocity = 180;
        break;
      case '7':
        velocity = 200;
        break;
      case 'B':
        detachInterrupt( Interrupt_zero );
        detachInterrupt( Interrupt_one );
        digitalWrite(Dir_Right_M , LOW);
        digitalWrite( Dir_Left_M , HIGH);
        analogWrite( PWM_Left_M , velocity);
        analogWrite( PWM_Right_M , velocity);
        break;
      case 'F':
        attachInterrupt( Interrupt_zero , Right_encoder , RISING);
        //attachInterrupt( Interrupt_one , Left_encoder , RISING);
        digitalWrite(Dir_Right_M , HIGH);
        digitalWrite( Dir_Left_M , LOW);
        analogWrite( PWM_Left_M , velocity);
        analogWrite( PWM_Right_M , velocity);
        break ;
      case 'R':
        detachInterrupt( Interrupt_one );
        detachInterrupt( Interrupt_zero);
        digitalWrite(Dir_Right_M , LOW);
        digitalWrite( Dir_Left_M , LOW);
        analogWrite( PWM_Left_M , rot_velocity);
        analogWrite( PWM_Right_M , rot_velocity);
        break;
      case 'L':
        detachInterrupt( Interrupt_one );
        detachInterrupt( Interrupt_zero);
        digitalWrite(Dir_Right_M , HIGH);
        digitalWrite( Dir_Left_M , HIGH);
        analogWrite( PWM_Left_M , rot_velocity );
        analogWrite( PWM_Right_M , rot_velocity);
        break;
      default:
        detachInterrupt( Interrupt_zero );
        detachInterrupt( Interrupt_one );
        analogWrite( PWM_Left_M , LOW);
        analogWrite( PWM_Right_M , LOW);
        break;
    }
  }

  if (yaw > 180) yaw -= 360;
  else if (yaw < -180) yaw += 360;

  if (yaw < 75 && yaw > -75) {//Forward
    if (justTurned == false) {
      linear_pulsesYold = linear_pulses ;
      justTurned = true;
    }
    distY += ((linear_pulses - linear_pulsesYold) * 0.0222222222222222) ;
    linear_pulsesYold = linear_pulses ;
  } else if ((yaw > 75 && yaw < 130) || (yaw > -130 && yaw < -75)) { // hayd5ol hna lma ykon laff 90 fa lw meshy odam hayzwed fel column
    //Serial.println(linear_pulses);
    if (justTurned == true) {
      linear_pulsesXold = linear_pulses;
      justTurned = false;
    }
    distX += ((linear_pulses - linear_pulsesYold) * 0.0222222222222222);
    linear_pulsesXold = linear_pulses;
  }


  else if ( (yaw > 130 && yaw < 190) || (yaw > -190 && yaw < -130)) { // l hay5ly l direction backward l condtions bta3t l 180
    if (justTurned == false) {
      linear_pulsesYold = linear_pulses ;
      justTurned = true;
    }
    distY -= ((linear_pulses - linear_pulsesYold) * 0.0222222222222222);
    linear_pulsesYold = linear_pulses ;
  }
  //Serial.print("   \n");
  delay((timeStep * 1000) - (millis() - timer));
}

void conc() {
  while (connection == false) {
    if (Serial.available() > 0) {
      byte b = Serial.read();
      if (b == 'A') {
        delay(200);
        Serial.println('A');
        connection = true;
        break;
      }
    }
  }
}

bool detectorNoise() {
  bool detectorReading = digitalRead(LDetector) || digitalRead(RDetector);
  if (detectorReading == HIGH && prev_detectorReading == LOW) {
    prev_detectorReading = HIGH;
  } else if (detectorReading == HIGH && prev_detectorReading == HIGH) {
    if (millis() - Htimer > 35) {
      digitalWrite( Buzzerr , LOW);
      if ( count == 0) {
        now = millis();
        count = 1;
      }
      return false;
    } else {
      digitalWrite( Buzzerr , HIGH);
      return true;
    }
  } else if (detectorReading == LOW && prev_detectorReading == LOW) {
    Htimer = millis();
  } else if (detectorReading == LOW && prev_detectorReading == HIGH) {
    prev_detectorReading = LOW;
  }
}

