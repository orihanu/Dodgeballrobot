#include <LiquidCrystal.h>

// =====================================================
// LCD PINS
// =====================================================

const int lcdRS = A4;
const int lcdEN = A5;
const int lcdD4 = 4;
const int lcdD5 = 3;
const int lcdD6 = 2;
const int lcdD7 = 13;

LiquidCrystal lcd(
  lcdRS,
  lcdEN,
  lcdD4,
  lcdD5,
  lcdD6,
  lcdD7
);

// =====================================================
// HC-SR04 SENSOR PINS
// =====================================================

// Middle sensor
const int trigMiddle = 12;
const int echoMiddle = 11;

// Right sensor
const int trigRight = A0;
const int echoRight = A1;

// Left sensor
const int trigLeft = A2;
const int echoLeft = A3;

// Danger distance in centimeters
const int distanceLimit = 20;

// =====================================================
// L298N MOTOR PINS
// =====================================================

const int enA = 10;
const int in1 = 9;
const int in2 = 8;

const int in3 = 7;
const int in4 = 6;
const int enB = 5;

const int motorSpeed = 255;

// =====================================================
// VARIABLES
// =====================================================

int escapeDirection = 0;

String previousLCDLine1 = "";
String previousLCDLine2 = "";

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(9600);

  // ---------- LCD ----------
  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("ROBOT STARTING");

  lcd.setCursor(0, 1);
  lcd.print("PLEASE WAIT");

  // ---------- SENSORS ----------
  pinMode(trigMiddle, OUTPUT);
  pinMode(echoMiddle, INPUT);

  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);

  pinMode(trigLeft, OUTPUT);
  pinMode(echoLeft, INPUT);

  // Keep trigger pins LOW initially
  digitalWrite(trigMiddle, LOW);
  digitalWrite(trigRight, LOW);
  digitalWrite(trigLeft, LOW);

  // ---------- MOTORS ----------
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  stopMotors();

  randomSeed(micros());

  delay(1500);

  updateLCD(
    "EEE MODE",
    "SENSORS ACTIVE"
  );

  delay(1000);
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  // Read sensors one by one to reduce interference
  long middleDistance = measureDistance(
    trigMiddle,
    echoMiddle
  );

  delay(40);

  long rightDistance = measureDistance(
    trigRight,
    echoRight
  );

  delay(40);

  long leftDistance = measureDistance(
    trigLeft,
    echoLeft
  );

  delay(40);

  // ---------- SERIAL MONITOR ----------
  Serial.print("Middle: ");
  printDistance(middleDistance);

  Serial.print(" | Right: ");
  printDistance(rightDistance);

  Serial.print(" | Left: ");
  printDistance(leftDistance);

  Serial.println();

  // ---------- DETECTION ----------
  bool middleDetected =
    middleDistance > 0 &&
    middleDistance < distanceLimit;

  bool rightDetected =
    rightDistance > 0 &&
    rightDistance < distanceLimit;

  bool leftDetected =
    leftDistance > 0 &&
    leftDistance < distanceLimit;

  // ===================================================
  // NO SENSOR DETECTS AN OBJECT
  // ===================================================

  if (
    !middleDetected &&
    !rightDetected &&
    !leftDetected
  ) {
    escapeDirection = 0;

    stopMotors();

    updateLCD(
      "EEE MODE",
      "NO DANGER"
    );
  }

  // ===================================================
  // ONLY MIDDLE SENSOR DETECTS AN OBJECT
  // ===================================================

  else if (
    middleDetected &&
    !rightDetected &&
    !leftDetected
  ) {
    // Choose direction once
    if (escapeDirection == 0) {
      escapeDirection = random(1, 3);
    }

    if (escapeDirection == 1) {
      moveRight();

      updateLCD(
        "FRONT DANGER",
        "DODGE RIGHT"
      );

      Serial.println("Action: DODGE RIGHT");
    }
    else {
      moveLeft();

      updateLCD(
        "FRONT DANGER",
        "DODGE LEFT"
      );

      Serial.println("Action: DODGE LEFT");
    }
  }

  // ===================================================
  // RIGHT SENSOR DETECTS AN OBJECT
  // MOVE LEFT
  // ===================================================

  else if (
    rightDetected &&
    !leftDetected
  ) {
    escapeDirection = 0;

    moveLeft();

    updateLCD(
      "RIGHT DANGER",
      "MOVE LEFT"
    );

    Serial.println("Action: MOVE LEFT");
  }

  // ===================================================
  // LEFT SENSOR DETECTS AN OBJECT
  // MOVE RIGHT
  // ===================================================

  else if (
    leftDetected &&
    !rightDetected
  ) {
    escapeDirection = 0;

    moveRight();

    updateLCD(
      "LEFT DANGER",
      "MOVE RIGHT"
    );

    Serial.println("Action: MOVE RIGHT");
  }

  // ===================================================
  // BOTH SIDES DETECT OBJECTS
  // ===================================================

  else if (
    leftDetected &&
    rightDetected
  ) {
    escapeDirection = 0;

    stopMotors();

    updateLCD(
      "BOTH SIDES",
      "STOP"
    );

    Serial.println("Action: STOP");
  }

  delay(50);
}

// =====================================================
// MEASURE DISTANCE
// =====================================================

long measureDistance(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(3);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(
    echoPin,
    HIGH,
    30000UL
  );

  // No echo received
  if (duration == 0) {
    return 999;
  }

  long distance = duration * 0.0343 / 2.0;

  return distance;
}

// =====================================================
// PRINT DISTANCE
// =====================================================

void printDistance(long distance) {
  if (distance == 999) {
    Serial.print("NO ECHO");
  }
  else {
    Serial.print(distance);
    Serial.print(" cm");
  }
}

// =====================================================
// MOVE LEFT
// Original motor logic
// =====================================================

void moveLeft() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);

  analogWrite(enA, motorSpeed);
  analogWrite(enB, motorSpeed);
}

// =====================================================
// MOVE RIGHT
// Original motor logic
// =====================================================

void moveRight() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);

  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);

  analogWrite(enA, motorSpeed);
  analogWrite(enB, motorSpeed);
}

// =====================================================
// STOP MOTORS
// =====================================================

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);

  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

// =====================================================
// LCD UPDATE
// =====================================================

void updateLCD(String line1, String line2) {
  line1 = line1.substring(0, 16);
  line2 = line2.substring(0, 16);

  if (
    line1 == previousLCDLine1 &&
    line2 == previousLCDLine2
  ) {
    return;
  }

  previousLCDLine1 = line1;
  previousLCDLine2 = line2;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(line1);

  lcd.setCursor(0, 1);
  lcd.print(line2);
}