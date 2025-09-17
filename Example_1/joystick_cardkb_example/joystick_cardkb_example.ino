//v2.2 -- joystick + cardkb --> funcitonal asdf keys + joystick to move, button to change colour. 
// hannah 16-09-2025 
#include <M5Core2.h>
#include <Wire.h> 

#define JOY_ADDR   0x52
#define CARDKB_ADDR 0x5F

// block/player location and colour 
int playerX = 160, playerY = 120;
uint16_t playerColor = RED;

//calibration joystick 
uint8_t joyCenterX = 128, joyCenterY = 128;
int lastJoyBtn = 1;

// CardKB state (stolen directly from example)
bool cardKBAvailable = false;
String kbLog = "";

//---------------------------------------

void setup() {
  M5.begin();
  Serial.begin(115200);
  Wire.begin(32, 33, 400000UL); // SDA=32, SCL=33

  // screen setup
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(40, 0);
  M5.Lcd.println("GameBoy + CardKB");

  // calibrate joystick
  Wire.requestFrom(JOY_ADDR, 3);
  if (Wire.available() >= 3) {
    joyCenterX = Wire.read();
    joyCenterY = Wire.read();
    Wire.read();
    Serial.printf("Joystick center: X=%d Y=%d\n", joyCenterX, joyCenterY);
  }

  // Scan i2c devices because dual button != i2c
  scanI2C();
}

void loop() {
  M5.update();

  // Track if anything changed this frame
  bool needsRedraw = false;

  // read joystick
  static uint8_t joyX=0, joyY=0, joyBtn=0;
  Wire.requestFrom(JOY_ADDR, 3);
  if (Wire.available() >= 3) {
    joyX = Wire.read();
    joyY = Wire.read();
    joyBtn = Wire.read();
  }

  // movement (joystick)
  int deadzone = 10;
  int moveX = 0, moveY = 0;
  if (abs(joyX - joyCenterX) > deadzone) moveX = map(joyX, 0, 255, 5, -5);
  if (abs(joyY - joyCenterY) > deadzone) moveY = map(joyY, 0, 255, -5, 5);

  int newPlayerX = constrain(playerX + moveX, 0, 300);
  int newPlayerY = constrain(playerY + moveY, 20, 220);

  // Check if position changed
  if (newPlayerX != playerX || newPlayerY != playerY) {
    playerX = newPlayerX;
    playerY = newPlayerY;
    needsRedraw = true;
  }

  // Joystick button toggles color
  if (joyBtn == 0 && lastJoyBtn == 1) {
    playerColor = (playerColor == RED) ? BLUE : RED;
    needsRedraw = true;
    Serial.println("Joystick button pressed: color toggled");
  }
  lastJoyBtn = joyBtn;

  // --- Read CardKB if available ---
  if (cardKBAvailable) {
    Wire.requestFrom(CARDKB_ADDR, 1);
    while (Wire.available()) {
      char c = Wire.read();
      if (c != 0) {
        Serial.printf("CardKB key: %c (0x%02X)\n", c, c);
        bool playerMoved = false;

        // Handle controls
        if (c == 'w' || c == 'W') {
          int newY = max(20, playerY - 5);
          if (newY != playerY) {
            playerY = newY;
            playerMoved = true;
          }
        }
        else if (c == 's' || c == 'S') {
          int newY = min(220, playerY + 5);
          if (newY != playerY) {
            playerY = newY;
            playerMoved = true;
          }
        }
        else if (c == 'a' || c == 'A') {
          int newX = max(0, playerX - 5);
          if (newX != playerX) {
            playerX = newX;
            playerMoved = true;
          }
        }
        else if (c == 'd' || c == 'D') {
          int newX = min(300, playerX + 5);
          if (newX != playerX) {
            playerX = newX;
            playerMoved = true;
          }
        }
        else if (c == ' ') {
          playerColor = (playerColor == RED) ? BLUE : RED;
          needsRedraw = true;
          Serial.println("CardKB Space pressed: color toggled");
        }

        if (playerMoved) needsRedraw = true;

        // Keep last 20 keys
        kbLog += c;
        if (kbLog.length() > 20) kbLog.remove(0, kbLog.length() - 20);
        needsRedraw = true; // Text changed, need redraw
      }
    }
  }

  // --- Draw only if something changed ---
  if (needsRedraw) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.fillRect(playerX, playerY, 20, 20, playerColor);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(5, 5);
    M5.Lcd.printf("JX:%3d JY:%3d JB:%d", joyX, joyY, joyBtn);

    M5.Lcd.setCursor(5, 20);
    M5.Lcd.printf("Keys: %s", kbLog.c_str());
  }

  delay(30);
}



//-------------------------------------------

//debugging
void scanI2C() {
  Serial.println("Scanning I2C bus...");
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at 0x%02X\n", address);
      if (address == CARDKB_ADDR) {
        cardKBAvailable = true;
        Serial.println("CardKB detected!");
      }
      nDevices++;
    } 
    else if (error == 4) {
      Serial.printf("Unknown error at 0x%02X\n", address);
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found");
  Serial.println("I2C scan complete.\n");
}
