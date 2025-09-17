/**
 * Based on the example by M5Stack & Example codes 
 * ---------------------------------------------
 * M5STACK example: Unit_encoder -->  Copyright (c) 2021 by M5Stack
 * ---------------------------------------------
 * Hardware: 
 * Uses: M5Stack Core (Core/Basic) + Unit Hub (I2C) with Joystick (0x52) + Unit Encoder (I2C)
 * ---------------------------------------------
 * edittedd by hannah 17-9-2025 
 * selftracker: v1.2 
 */

#include <M5Stack.h>
#include "Unit_Encoder.h"
#include <Wire.h>

#define JOY_ADDR    0x52    // joystick Unit I2C address --> you can always check the address by running the code snippet 'tell me your address' found in the github

// Encoder unit uses default address inside library (0x40). encoder.begin() should work.
Unit_Encoder encoder;      // unit_Encoder object

//player/block state
int playerX = 160;
int playerY = 120;
int playerSize = 20;

//colour palette (RGB565) --> playerblock will rotate between the rainbow! 
const uint16_t colours[] = {
  0xF800, // Red
  0xFD20, // Orange
  0xFFE0, // Yellow
  0x07E0, // Green
  0x001F, // Blue
  0x8010, // Purple
  0xFB56  // Pink
};
const size_t COLOUR_COUNT = sizeof(colours) / sizeof(colours[0]);
size_t colourIndex = 0;

// Joystick calibration
uint8_t joyCenterX = 128;
uint8_t joyCenterY = 128;

//track previous button states (for edge detection)
int lastJoyBtn = 1;         // joystick button: 0 = pressed, 1 = released
int lastEncBtnRaw = 1;      // encoder button raw: library returns 1/0 (example usage shows 0 = pressed)

void setup() {
  M5.begin();
  Wire.begin();            // default I2C pins for the board

  // Init encoder library (uses I2C inside). If the board you are using board needs explicit pins, replace with line below, but for basic/core encoder begin works
  // encoder.begin(&Wire, 0x40, SDA_PIN, SCL_PIN); 
  encoder.begin();

  // screen
   M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(60, 0);
  M5.Lcd.println("Game Boy Demo");

  //calibrate joystick center
  Wire.requestFrom(JOY_ADDR, (uint8_t)3);
  if (Wire.available() >= 3) {
    joyCenterX = Wire.read();
    joyCenterY = Wire.read();
    Wire.read(); // discard button byte
  }
  delay(50);
}


void loop() {
  M5.update();

  //read joystick 
  static uint8_t joyX = 128, joyY = 128, joyBtn = 1;
  Wire.requestFrom(JOY_ADDR, (uint8_t)3);
  if (Wire.available() >= 3) {
    joyX = Wire.read();
    joyY = Wire.read();
    joyBtn = Wire.read();  // 0 = pressed, 1 = released 
  }

  //read encoder
  signed short encCount = encoder.getEncoderValue();   
  int encBtnRaw = encoder.getButtonStatus();           // library examples treat 0 as pressed, 1 as released

  //using encoder to change player size  scale encCount down so changes are reasonable and controllable
  playerSize = constrain(20 + (encCount / 4), 10, 80);

  //deadzone and mapping for joystick
  const int deadzone = 10;
  int moveX = 0, moveY = 0;
  if (abs((int)joyX - (int)joyCenterX) > deadzone) {
    moveX = map(joyX, 0, 255, 5, -5);
  }
  if (abs((int)joyY - (int)joyCenterY) > deadzone) {
    moveY = map(joyY, 0, 255, -5, 5);
  }
  playerX = constrain(playerX + moveX, 0, 320 - playerSize);
  playerY = constrain(playerY + moveY, 0, 240 - playerSize);

  // joystick button + colours
  if (lastJoyBtn == 1 && joyBtn == 0) {
    colourIndex = (colourIndex + 1) % COLOUR_COUNT;
  }
  lastJoyBtn = joyBtn;

  //encoder button  ---> as stated earlier, some library examples use getButtonStatus() where 0 == pressed. We follow that convention for clarity 
  if (lastEncBtnRaw == 1 && encBtnRaw == 0) {
    colourIndex = (colourIndex + 1) % COLOUR_COUNT;
  }
  lastEncBtnRaw = encBtnRaw;

  //everything in screeeen 
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.fillRect(playerX, playerY, playerSize, playerSize, colours[colourIndex]);

  //some stuff to debug, mostly printitng the raw values
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(5, 5);
  M5.Lcd.printf("JX:%3d JY:%3d JB:%d ENC:%d EB:%d", joyX, joyY, joyBtn, encCount, encBtnRaw);   // Print: Joystick X,Y, joystick btn, encoder count, encoder btn raw


  delay(30); // ~30 FPS
}
