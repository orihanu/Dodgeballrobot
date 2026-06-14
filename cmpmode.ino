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
// L298N MOTOR DRIVER PINS
// =====================================================

const int enA = 10;
const int in1 = 9;
const int in2 = 8;

const int in3 = 7;
const int in4 = 6;
const int enB = 5;

const int motorSpeed = 200;

// =====================================================
// VARIABLES
// =====================================================

String receivedCommand = "";

String previousLCDLine1 = "";
String previousLCDLine2 = "";

// =====================================================
// SETUP
// =====================================================

void setup() {
  Serial.begin(9600);

  // Very short timeout for faster command reception
  Serial.setTimeout(5);

  // ---------- LCD ----------
  lcd.begin(16, 2);
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CMP MODE");

  lcd.setCursor(0, 1);
  lcd.print("CAMERA READY");

  // ---------- MOTORS ----------
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  stopMotors();

  delay(1000);

  updateLCD(
    "CMP MODE",
    "WAITING CMD"
  );
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {
  readSerialCommands();
}

// =====================================================
// READ SERIAL COMMANDS
// Reads all available commands and uses the newest one
// =====================================================

void readSerialCommands() {
  while (Serial.available() > 0) {
    String newCommand = Serial.readStringUntil('\n');

    newCommand.trim();
    newCommand.toUpperCase();

    if (newCommand.length() > 0) {
      receivedCommand = newCommand;
    }
  }

  if (receivedCommand.length() > 0) {
    Serial.print("Received: ");
    Serial.println(receivedCommand);

    runCommand(receivedCommand);

    // Process each received command only once
    receivedCommand = "";
  }
}

// =====================================================
// COMMAND CONTROL
// =====================================================

void runCommand(const String &command) {
  if (command == "MODE_CMP") {
    stopMotors();

    updateLCD(
      "CMP MODE",
      "CAMERA ACTIVE"
    );
  }

  else if (command == "LEFT") {
    moveLeft();

    updateLCD(
      "CMP MODE",
      "MOVE LEFT"
    );
  }

  else if (command == "RIGHT") {
    moveRight();

    updateLCD(
      "CMP MODE",
      "MOVE RIGHT"
    );
  }

  else if (command == "CENTER") {
    stopMotors();

    updateLCD(
      "CMP MODE",
      "BALL CENTERED"
    );
  }

  else if (command == "DODGE") {
    performDodge();
  }

  else if (command == "NO_BALL") {
    stopMotors();

    updateLCD(
      "BALL NOT FOUND",
      "ROBOT STOPPED"
    );
  }

  else if (command == "STOP") {
    stopMotors();

    updateLCD(
      "CMP MODE",
      "ROBOT STOPPED"
    );
  }

  else {
    stopMotors();

    updateLCD(
      "UNKNOWN COMMAND",
      command
    );
  }
}

// =====================================================
// NON-BLOCKING DODGE
// No delay here
// =====================================================

void performDodge() {
  moveRight();

  updateLCD(
    "DODGING...",
    "MOVE RIGHT"
  );

  /*
    Motor continues moving until Python sends:
    NO_BALL, STOP, CENTER, LEFT, or RIGHT.

    There is no delay(500), so NO_BALL can stop
    the motor immediately.
  */
}

// =====================================================
// MOVE LEFT
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
  // Disable motors immediately
  analogWrite(enA, 0);
  analogWrite(enB, 0);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
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