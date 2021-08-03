#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

byte CUSTOM_CHARACTER_DEGREE[8] = {
  B00110,
  B01001,
  B01001,
  B00110,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte CUSTOM_CHARACTER_FAN_ON[8] = {
  B00100,
  B10101,
  B01110,
  B11111,
  B01110,
  B10101,
  B00100,
  B00000,
};

byte CUSTOM_CHARACTER_FAN_OFF[8] = {
  B00100,
  B00100,
  B00100,
  B11111,
  B00100,
  B00100,
  B00100,
  B00000,
};

enum COMPONENTS {
    FAN = 22,
    SENSOR = 28,
    SPEAKER = 34,
    LIGHT = 44,
    UP_BUTTON = 46,
    DOWN_BUTTON = 52,
};

void Configure_IO() {
    Output(COMPONENTS::FAN);
    Output(COMPONENTS::LIGHT);
    Output(COMPONENTS::SPEAKER);
    Input(COMPONENTS::SENSOR);
    Input(COMPONENTS::UP_BUTTON);
    Input(COMPONENTS::DOWN_BUTTON);
}

void Output(int Component) {
    pinMode(Component, OUTPUT);
}

void Input(int Component) {
    pinMode(Component, INPUT);
}

struct LCD_COLUMN_POSITIONS{
    const byte FIRST = 0;
    const byte MIDDLE = 8;
    const byte LAST = 15;
};

struct LCD_ROW_POSITIONS{
    const byte TOP = 0;
    const byte BOTTOM = 1;
};

struct LCD_DIMENSIONS{
    LCD_COLUMN_POSITIONS COLUMNS;
    LCD_ROW_POSITIONS ROWS;
};

LCD_DIMENSIONS SCREEN;

void PrintAtPosition(char Message[], byte Y, byte X) {
    const char LINE_END = '\0';
    byte character = 0;

    while (Message[character] != LINE_END) {
        lcd.setCursor(constrain(X + character, SCREEN.COLUMNS.FIRST, SCREEN.COLUMNS.LAST), constrain(Y, SCREEN.ROWS.TOP, SCREEN.ROWS.BOTTOM));
        lcd.print(Message[character]);
        
        character++;
    }
}

void PrintAtPosition(int number, byte Y, byte X) {
    lcd.setCursor(constrain(X, SCREEN.COLUMNS.FIRST, SCREEN.COLUMNS.LAST), constrain(Y, SCREEN.ROWS.TOP, SCREEN.ROWS.BOTTOM));
    lcd.print(number);
}

void WriteAtPosition(byte CustomChar[], byte Y, byte X) {
    const byte FIRST_SLOT = 0;
    const byte LAST_SLOT = 7;
    static byte slot;

    slot = constrain(slot, FIRST_SLOT, LAST_SLOT);

    lcd.createChar(slot, CustomChar);
    lcd.setCursor(constrain(X, SCREEN.COLUMNS.FIRST, SCREEN.COLUMNS.LAST), constrain(Y, SCREEN.ROWS.TOP, SCREEN.ROWS.BOTTOM));
    lcd.write((byte)slot);
    slot++;
}

void Setup_LCD() {
    lcd.begin(16, 2);
    PrintAtPosition("Current:", SCREEN.ROWS.TOP, SCREEN.COLUMNS.FIRST);
    PrintAtPosition("Set:", SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.FIRST);
    PrintAtPosition("C", SCREEN.ROWS.TOP, SCREEN.COLUMNS.MIDDLE + 3);
    PrintAtPosition("C", SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.MIDDLE + 3);
    WriteAtPosition(CUSTOM_CHARACTER_DEGREE, SCREEN.ROWS.TOP, SCREEN.COLUMNS.MIDDLE + 2);
    WriteAtPosition(CUSTOM_CHARACTER_DEGREE, SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.MIDDLE + 2);
}

boolean Beeped = false;

struct Temps {
    const byte BUFFER = 2;
    int current;
    int target;
};

Temps temp;

void setup(){
    Serial.begin(9600);
    Configure_IO();
    Setup_LCD();
    temp.target = 55;
}

void loop(){
    temp.current = constrain(analogRead(COMPONENTS::SENSOR), 0, 99);

    PrintAtPosition(temp.current, SCREEN.ROWS.TOP, SCREEN.COLUMNS.MIDDLE);

    if (IsPressed(COMPONENTS::DOWN_BUTTON)) {
        temp.target--;
    }

    if (IsPressed(COMPONENTS::UP_BUTTON)) {
        temp.target++;
    }

    PrintAtPosition(temp.target, SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.MIDDLE);

    if (abs(temp.target - temp.current) > temp.BUFFER) {
        if (temp.current > temp.target) {
            WriteAtPosition(CUSTOM_CHARACTER_FAN_ON, SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.LAST);
            beep();
            toggleFan(HIGH);
            toggleLight(LOW);
        }
        else {
            WriteAtPosition(CUSTOM_CHARACTER_FAN_OFF, SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.LAST);
            toggleFan(LOW);
            toggleLight(HIGH);
        }
    }
    else {
        WriteAtPosition(CUSTOM_CHARACTER_FAN_OFF, SCREEN.ROWS.BOTTOM, SCREEN.COLUMNS.LAST);
        toggleFan(LOW);
        toggleLight(LOW);
    }
}

boolean IsPressed(int button) {
    const byte MIN_CONSECUTIVE_READS = 5;

    for (byte reads = 1; reads <= MIN_CONSECUTIVE_READS; reads++) {
       if (digitalRead(button) == LOW) {
            return false;
        }  
    }
    return true;
}

unsigned long now() {
    return millis();
}

void beep() {
    const int beepTime = 500;
    static unsigned long lastQuiet;
    
    if (now() - lastQuiet <= beepTime) {
        tone(COMPONENTS::SPEAKER, 400);
    }
    else {
        lastQuiet = now();
        noTone(COMPONENTS::SPEAKER);
    }
}

void toggleFan(boolean on) {
    static byte percentOn;

    if (on) {
        if (percentOn < 100) {
            analogWrite(COMPONENTS::FAN, map(percentOn, 0, 100, 0, 255));
            percentOn++;
        }
        else {
            digitalWrite(COMPONENTS::FAN, HIGH);
        }
    }
    else {
        if (percentOn > 0) {
            analogWrite(COMPONENTS::FAN, map(percentOn, 0, 100, 0, 255));
            percentOn--;
        }
        else {
            digitalWrite(COMPONENTS::FAN, LOW);
        }
    }
}

void toggleLight(boolean on) {
    if (on) {
        digitalWrite(COMPONENTS::FAN, HIGH);
    }
    else {
        digitalWrite(COMPONENTS::FAN, LOW);
    }
}
