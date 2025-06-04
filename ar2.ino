#include <Servo.h>

Servo myservo1;
Servo myservo2;

const int servoPin1 = 9;
const int servoPin2 = 10;

const int pin3 = 3;  // 부저
const int pin7 = 7;
const int pin8 = 8;

unsigned long lastMoveTime = 0;
bool isAt180 = true;
bool carDetected = false;
bool overrideByCar = false;

bool buzzDone = false;
unsigned long buzzStartTime = 0;

// 부저 토글 관련 변수
unsigned long lastBuzzToggleTime = 0;
bool isBuzzerOn = false;

void setup() {
  Serial.begin(9600);

  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);

  pinMode(pin3, OUTPUT);
  pinMode(pin7, OUTPUT);
  pinMode(pin8, OUTPUT);

  myservo1.write(172);
  myservo2.write(172);
  lastMoveTime = millis();

  digitalWrite(pin3, LOW);
  digitalWrite(pin7, LOW);
  digitalWrite(pin8, LOW);
}

void loop() {
  unsigned long currentTime = millis();

  // 시리얼 명령 수신
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'C') {
      if (!carDetected) {
        buzzStartTime = millis();  // 부저 시작 시점
        buzzDone = false;
      }
      carDetected = true;
    } else if (command == 'N') {
      carDetected = false;
      buzzDone = false;
      isBuzzerOn = false;
      digitalWrite(pin3, LOW);
    }
  }

  // 차량이 감지된 경우
  if (isAt180 && carDetected) {
    myservo1.write(80);
    myservo2.write(80);
    overrideByCar = true;

    digitalWrite(pin7, HIGH);
    digitalWrite(pin8, LOW);

    lastMoveTime = millis();
    isAt180 = false;
  }

  // 차량 감지가 끝난 경우
  else if (overrideByCar && !carDetected) {
    myservo1.write(172);
    myservo2.write(172);
    isAt180 = true;
    overrideByCar = false;

    digitalWrite(pin3, LOW);
    digitalWrite(pin7, LOW);
    digitalWrite(pin8, HIGH);

    lastMoveTime = millis();
  }

  // 차량 감지 외 일반 주기적 동작
  else if (!overrideByCar) {
    if (isAt180 && currentTime - lastMoveTime >= 30000) {
      myservo1.write(80);
      myservo2.write(80);
      isAt180 = false;
      lastMoveTime = currentTime;
    } else if (!isAt180 && currentTime - lastMoveTime >= 10000) {
      myservo1.write(172);
      myservo2.write(172);
      isAt180 = true;
      lastMoveTime = currentTime;
    }

    if (!isAt180) {
      digitalWrite(pin3, LOW);
      digitalWrite(pin7, LOW);
      digitalWrite(pin8, LOW);
    }
  }

  // 차량 감지 중일 때 부저를 0.25초 간격으로 토글
  if (carDetected && !buzzDone) {
    if (millis() - lastBuzzToggleTime >= 250) {
      isBuzzerOn = !isBuzzerOn;
      digitalWrite(pin3, isBuzzerOn ? HIGH : LOW);
      lastBuzzToggleTime = millis();
    }
  }
}
