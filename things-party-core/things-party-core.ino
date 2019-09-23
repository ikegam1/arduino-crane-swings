#include <SoftwareSerial.h>
#include <Servo.h>
//#include <MsTimer2.h>
//#include <TimerOne.h>
#include "config.h"
#include "env.h"
// Adafruit Motor Shild Libralyより
#include <AFMotor.h>

//BLEシリアルのPIN
SoftwareSerial BTserial(BTSerialRX,BTSerialTX); // RX, TX

// DCモーターのM4
// クレーンの左右
// わかりづらくなってしまったけど、M3がmotor4
AF_DCMotor motor4(3);
// HC-SR04の数値
long durationA;
int distanceA;

// DCモーターのM3
// クレーンの巻き上げ
AF_DCMotor motor3(4);
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

  OpenArm();
  servoA.write(0);
  delay(100);
  
  //timer
  //Timer1.initialize(1000000);
  //Timer1.attachInterrupt(LedBlink);
  //MsTimer2::set(1000, LedBlink);
  //MsTimer2::start();
}

int led = 11; //R=100, G=10, B = 1 while=111, yellow=110,  cyan=11
int recentDistanceA = 0;
int recentDistanceB = 0;
//床に置かれているお菓子などが超音波センサーに狂いを生じさせる。
//移動量は固定にして対応
int downCnt = 0;
int upCnt = 0; 
uint8_t i;
String input;
void loop() {
    if (BTserial.available() > 0) { // 受信したデータが存在する
        Serial.println("Receive:");
        // 改行コード(10)を検出したら、そこまでの文字列を取得
        input = BTserial.readStringUntil(10);
        Serial.println(input);
        i = input[0] - '0';
        if ( i >= 0 ){
          f = i;
          Serial.print("Receive:");
          Serial.println(f);
        }
    }
    
    LedBlink();
    //f=FlgRotate; //debug
        
    //start前
    if(f==FlgStart){
      led = 111;
      downCnt = 0;
      upCnt = 0;
      servoB.write(0);
      servoB.write(180);
      delay(10);
      return;
    }

    //Serial.print("mode=");
    //Serial.println(f);
    
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
      for (i=0; i<=180; i+=1) {
        motor4.setSpeed(i);
        delay(50);
      }
      delay(10);
    
      recentDistanceA = distanceA;
    }

    //stop
    if(f==FlgStop){
      led = 0;
      //Serial.print("mode=FlgStop");
      motor4.run(RELEASE);
      delay(10);      
    }

    //アーム角度
    if(f==FlgRotate){
      led = 10;
      //Serial.println("mode=FlgRotate");
      Serial.println(posA);
      posA+=1;
      if(posA >= 360){
        posA = 0;
        i = 0;
      }else if(posA > 180){
        i = 180 - (posA - 180);
      }else{
        i = posA;
      }
      servoA.write(i);
      delay(20);
    }

    if(f==FlgDown){
      GetDistanceB();
      Serial.print("mode=FlgDown");
      delay(200);
      f=FlgCatch;
    }

    if(f==FlgCatch){
      led = 100;
      GetDistanceB();
      Serial.println("mode=FlgCatch");

      //誤差がおかしいときは何もしない
      if(abs(distanceB - recentDistanceB) > 100 && recentDistanceB != 0){
        Serial.print("abs(distanceB - recentDistanceB)");
        Serial.println(abs(distanceB - recentDistanceB));
        //return;
      }
      //地面に近づくとCatch
      downCnt+=1;
      Serial.print("downCnt=");
      Serial.println(downCnt);
      //if(distanceB <= limitMinPinB){
      if(downCnt >= 5){
        motor3.run(RELEASE);
        delay(1000);

        CloseArm();
        delay(2000);
        
        f = FlgUp;
        return;
      }

      //モーターを動かす アームダウン
      motor3.run(FORWARD);
      Serial.print("motor3 run");
      for (i=0; i<180; i+=1) {
        motor3.setSpeed(i);
        delay(30);
      }
      motor3.run(RELEASE);
      delay(10);
    
      recentDistanceB = distanceB;

    }

    //クレーンをUp
    if(f==FlgUp){
      led = 110;
      GetDistanceB();

      //誤差がおかしいときは何もしない
      if(abs(distanceB - recentDistanceB) > 100 && recentDistanceB != 0){
        Serial.print("abs(distanceB - recentDistanceB)");
        Serial.println(abs(distanceB - recentDistanceB));
        //return;
      }
      
      //地面はなれるまでUp
      upCnt+=1;
      Serial.print("upCnt=");
      Serial.println(upCnt);
      //if(distanceB >= limitMaxPinB){
      if(upCnt >= 3){
        motor3.run(RELEASE);
        delay(500);
        
        f = FlgLeft;
        return;
      }
      
      //モーターを動かす アームアップ
      motor3.run(BACKWARD);
      Serial.print("motor3 run");
      for (i=0; i<100; i+=1) {
        motor3.setSpeed(i);
        delay(12);
      }
      motor3.run(RELEASE);
      delay(10);
      recentDistanceB = distanceB;
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
        delay(1000);

        // アームをオープン
        OpenArm();
        delay(1000);
               
        f = FlgStart;
        return;
      }

      //モーターを動かす
      motor4.run(BACKWARD);
      Serial.print("motor4 run");
      for (i=0; i<100; i+=5) {
        motor4.setSpeed(i);
        delay(100);
      }
      delay(10);
    
      recentDistanceA = distanceA;
    }
}

int toggle = 1;
int timerCnt = 1;
int totalCnt = 1;
int _led = 0;
//点滅の間隔は1/100
void LedBlink() {
  totalCnt+=1;

  //3000Cnt毎にステータスを進める
  if(DEBUG == 2 && totalCnt % 3000 == 0){
    f+=1;
  }
  
  if(timerCnt < 100){
    timerCnt += 1;
    return;
  }else{
    if(DEBUG >= 1){
      Serial.print("flag = ");
      Serial.println(f);
      GetDistanceA();
      GetDistanceB();
      Serial.print("distansA=");
      Serial.println(distanceA);
      Serial.print("distansB=");
      Serial.println(distanceB);
    }
  }
  timerCnt = 1;

  _led = led;
  digitalWrite(LedRed, HIGH);
  digitalWrite(LedGreen, HIGH);
  digitalWrite(LedBlue, HIGH);

  if (_led >= 100 && toggle == 1){
      digitalWrite(LedRed, LOW);
      _led-=100;
  }
  if (_led >= 10 && toggle == 1){
      digitalWrite(LedGreen, LOW);
      _led-=10;
  }
  if (_led >= 1 && toggle == 1){
      digitalWrite(LedBlue, LOW);
  }

  toggle = 1 - toggle;
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
    digitalWrite(trigPinB, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPinB, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPinB, LOW);
    durationB = pulseIn(echoPinB, HIGH);
    distanceB= durationB*0.034/2;
}

void CloseArm(){
  for(posB = 180; posB>0; posB-=10)
  {
    servoB.write(posB);
    delay(100);
  }
}

void OpenArm(){
  for(posB = 0; posB<=180; posB+=10)
  {
    servoB.write(posB);
    delay(100);
  }
}
