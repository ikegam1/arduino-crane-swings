#include <SoftwareSerial.h>
#include <Servo.h>
#include <MsTimer2.h>
#include <TimerOne.h>
#include "config.h"
// Adafruit Motor Shild Libralyより
#include <AFMotor.h>

//BLEシリアルのPIN
SoftwareSerial BTserial(2,13); // RX, TX

// DCモーターのM4
// クレーンの左右
AF_DCMotor motor4(4);
// HC-SR04の数値
long durationA;
int distanceA;

// DCモーターのM3
// クレーンの巻き上げ
AF_DCMotor motor3(3);
// HC-SR04の数値
long durationB;
int distanceB;

// アーム回転サーボ
Servo servoA;
int posA = 0;

// アーム開閉サーボ
Servo servoB;
int posB = 0;

//アクションフラグ
//各アクションを判別する(*=input)
// 0 = スタート
// 1 = クレーン右 *
// 2 = 水平移動ストップ *
// 3 = アーム回転 *
// 4 = アームストップ
// 5 = クレーン下
// 6 =
// 7 = アーム閉じる＆クレーン上 *
// 8 = クレーン左
// 9 = クレーン左
int f=0;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
    
  // Define inputs and outputs:
  pinMode(trigPinA, OUTPUT);
  pinMode(echoPinA, INPUT);
  pinMode(trigPinB, OUTPUT);
  pinMode(echoPinB, INPUT);
  servoA.attach(ServoPinA);
  servoB.attach(ServoPinB);

  // turn on motor
  motor4.setSpeed(200);
  motor4.run(RELEASE);
  
  BTserial.begin(9600);  
  Serial.println("BTserial started at 9600");

  pinMode(LedGreen, OUTPUT);
  pinMode(LedBlue, OUTPUT);
  pinMode(LedRed, OUTPUT);
  
  //timer
  //Timer1.initialize(1000000);
  //Timer1.attachInterrupt(LedBlink);
  //MsTimer2::set(1000, LedBlink);
  //MsTimer2::start();
}

int led = 11; //R=100, G=10, B = 1 while=111, yellow=110,  cyan=11
int recentDistanceA = 0;
int recentDistanceB = 0;
uint8_t i;
String input;
void loop() {

    GetFlg();
    LedBlink();
    
    //start前
    if(f==FlgStart){
      led = 111;
      servoB.write(180);
      delay(10);
      return;
    }

    Serial.print("mode=");
    Serial.println(f);
    
    //右移動
    if(f==FlgRight){
      led = 1;
      GetDistanceA();
      Serial.println("mode=FlgRight");
      Serial.print("distanceA=");
      Serial.println(distanceA);
      //誤差がおかしいときは何もしない
      if(abs(distanceA - recentDistanceA) > 100 && recentDistanceA != 0){
        Serial.print("abs(distanceA - recentDistanceA)");
        Serial.println(abs(distanceA - recentDistanceA));
        return;
      }
      //limitを超えるとStop
      if(distanceA >= limitMaxPinA){
        motor4.run(RELEASE);
        delay(100);
        f = FlgStop;
        return;
      }

      //モーターを動かす
      motor4.run(FORWARD);
      Serial.print("motor4 run");
      for (i=0; i<100; i+=5) {
        motor4.setSpeed(i);
        delay(2);
      }
      delay(10);
    
      recentDistanceA = distanceA;
    }

    //stop
    if(f==FlgStop){
      led = 0;
      Serial.print("mode=FlgStop");
      motor4.run(RELEASE);
      delay(10);      
    }

    //回転
    if(f==FlgRotate){
      led = 10;
      Serial.println("mode=FlgRotate");
      posA = 50;
      servoA.write(posA);
      delay(50);
    }else{
      servoA.write(90);
    }

    if(f==FlgCatch){
      led = 100;
      GetDistanceB();
      Serial.print("mode=FlgCatch");

      //誤差がおかしいときは何もしない
      if(abs(distanceB - recentDistanceB) > 100 && recentDistanceB != 0){
        Serial.print("abs(distanceB - recentDistanceB)");
        Serial.println(abs(distanceB - recentDistanceB));
        return;
      }
      //地面に近づくとCatch
      if(distanceB >= limitMinPinB){
        motor3.run(RELEASE);
        delay(100);

        // アームをオープン
        for(posB = 0; posB>=90; posB+=1)
        {
          servoB.write(posB);
          delay(50);
        }
        delay(1000);
        // sweeps from 180 degrees to 0 degrees
        for(posB = 90; posB>=0; posB-=1)
        {
          servoB.write(posB);
          delay(50);
        }
        delay(2000);
        
        f = FlgUp;
        return;
      }

      //モーターを動かす
      motor3.run(FORWARD);
      Serial.print("motor3 run");
      for (i=0; i<100; i+=5) {
        motor3.setSpeed(i);
        delay(2);
      }
      delay(10);
    
      recentDistanceB = distanceB;

    }

    //クレーンをUp
    if(f==FlgUp){
      led = 110;
      GetDistanceB();

      //地面はなれるまでUp
      if(distanceB >= limitMaxPinB){
        motor3.run(RELEASE);
        delay(100);
        
        f = FlgLeft;
        return;
      }
      
      //モーターを動かす
      motor3.run(FORWARD);
      Serial.print("motor3 run");
      for (i=0; i<100; i+=5) {
        motor3.setSpeed(i);
        delay(2);
      }
      delay(10);
    }
    
    //左移動
    if(f==FlgLeft){
      led = 110;
      GetDistanceA();
      Serial.println("mode=FlgLeft");
      Serial.print("distanceA=");
      Serial.println(distanceA);
      //誤差がおかしいときは何もしない
      if(abs(distanceA - recentDistanceA) > 100 && recentDistanceA != 0){
        Serial.print("abs(distanceA - recentDistanceA)");
        Serial.println(abs(distanceA - recentDistanceA));
        return;
      }
      //limitを下回るとStop
      if(distanceA <= limitMinPinA){
        motor4.run(RELEASE);
        delay(100);

        // アームをオープン
        for(posB = 0; posB>=90; posB+=1)
        {
          servoB.write(posB);
          delay(50);
        }
        delay(1000);
         // アームをクローズ
        for(posB = 90; posB<=0; posB-=1)
        {
          servoB.write(posB);
          delay(50);
        }
               
        f = FlgStart;
        return;
      }

      //モーターを動かす
      motor4.run(FORWARD);
      Serial.print("motor4 run");
      for (i=0; i<100; i+=5) {
        motor4.setSpeed(i);
        delay(2);
      }
      delay(10);
    
      recentDistanceA = distanceA;
    }
}

int toggle = 1;
int timerCnt = 1;
//点滅の間隔は1/100
void LedBlink() {

  if(timerCnt < 100){
    timerCnt += 1;
    return;
  }
  timerCnt = 1;
  
  int _led = led;

  if (_led >= 100){
      digitalWrite(LedRed, LOW);
      _led-=100;
  }
  if (_led >= 10){
      digitalWrite(LedGreen, LOW);
      _led-=10;
  }
  if (_led >= 1){
      digitalWrite(LedBlue, LOW);
  }

  if (_led < 1 || toggle == 1){
      digitalWrite(LedRed, HIGH);
      digitalWrite(LedGreen, HIGH);
      digitalWrite(LedBlue, HIGH);    
  }
  toggle = 1 - toggle;
}

void GetFlg(){
    if (BTserial.available() > 0) { // 受信したデータが存在する
        Serial.println("Receive:");
        // 改行コード(10)を検出したら、そこまでの文字列を取得
        input = BTserial.readStringUntil(10);
        Serial.println(input);
        // 改行コード(10)を検出したら、そこまでの文字列を取得
        //input = BTserial.readStringUntil(10);
        i = input[0] - '0';
        if ( i >= 0 ){
          f = i;
          Serial.print("Receive:");
          Serial.println(f);
        }
    }
}

void GetDistanceA(){
    // 横方向のスタートからの距離を計測
    digitalWrite(trigPinA, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPinA, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinA, LOW);
    durationA = pulseIn(echoPinA, HIGH);
    distanceA= durationA*0.034/2;  
}

void GetDistanceB(){
    // 横方向のスタートからの距離を計測
    digitalWrite(trigPinA, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPinA, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinA, LOW);
    durationA = pulseIn(echoPinA, HIGH);
    distanceA= durationA*0.034/2;  
}
