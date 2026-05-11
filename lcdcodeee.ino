#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

String input = "";

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  lcd.print("READY");
}

void loop() {
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
    lcd.clear();

    if (input == "LEFT") {
      lcd.print("MOVE LEFT");
    }
    else if (input == "RIGHT") {
      lcd.print("MOVE RIGHT");
    }
    else if (input == "DODGE") {
      lcd.print("DODGING...");
      delay(500);
      lcd.clear();
      lcd.print("DODGED!");
    }
  }
}