#include <Servo.h>

Servo myservo1;
Servo myservo2;

const int servoPin1 = 9;
const int servoPin2 = 10;

const int pin11 = 11;  // 부저 핀 변경
const int pin7 = 7;
const int pin8 = 8;

unsigned long lastMoveTime = 0;
bool isAt180 = true;
bool carDetected = false;
bool overrideByCar = false;

bool buzzDone = false;
unsigned long buzzStartTime = 0;

// 부저 주파수 변경을 위한 변수
unsigned long lastBuzzChangeTime = 0;
int currentFrequency = 500;  // 초기 주파수 (500Hz)
const int frequencies[] = {500, 1000, 2000, 3000, 2000, 1000, 500};  // 사이렌 주파수 배열
int freqIndex = 0;  // 주파수 배열 인덱스

void setup() {
  Serial.begin(9600);

  myservo1.attach(servoPin1);
  myservo2.attach(servoPin2);

  pinMode(pin11, OUTPUT);  // 부저 핀을 출력 모드로 설정
  pinMode(pin7, OUTPUT);
  pinMode(pin8, OUTPUT);

  myservo1.write(172);
  myservo2.write(172);
  lastMoveTime = millis();

  digitalWrite(pin11, LOW);  // 부저 초기 상태
  digitalWrite(pin7, LOW);
  digitalWrite(pin8, LOW);
}

void loop() {
  unsigned long currentTime = millis();

  // 시리얼 명령 수신
  if (Serial.available()) {
    char command = Serial.read();
    if (command == 'C') {
      carDetected = true;
    } else if (command == 'N') {
      carDetected = false;
    }
  }

  // 차량이 감지된 경우
  if (isAt180 && carDetected) {
    myservo1.write(80);
    myservo2.write(80);
    overrideByCar = true;

    digitalWrite(pin7, HIGH);
    digitalWrite(pin11, HIGH);  // 부저도 HIGH (tone 제어됨)
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

    digitalWrite(pin11, LOW);  // 부저 끄기
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
    } else if (!isAt180 && currentTime - lastMoveTime >= 15000) {
      myservo1.write(172);
      myservo2.write(172);
      isAt180 = true;
      lastMoveTime = currentTime;
    }

    if (!isAt180) {
      digitalWrite(pin11, LOW);
      digitalWrite(pin7, LOW);
      digitalWrite(pin8, LOW);
    }
  }

  // 차량 감지된 상태에서만 부저 주파수 변경 (사이렌 효과)
  if (carDetected && currentTime - lastBuzzChangeTime >= 100) {  // 0.25초 간격으로 변경
    currentFrequency = frequencies[freqIndex];  // 현재 주파수 설정
    tone(pin11, currentFrequency);  // 부저 소리 변경
    freqIndex = (freqIndex + 1) % 5;  // 주파수 배열 순환 (500 -> 1000 -> 2000 -> 3000 -> 2000 -> 1000 -> 500)
    lastBuzzChangeTime = currentTime;
  } else if (!carDetected) {
    noTone(pin11);  // 차량이 감지되지 않으면 부저 끄기
  }
}
