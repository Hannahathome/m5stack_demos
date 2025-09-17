/*
 * M5Stack Core2 Heart Rate + SpO2 (MAX30100) – non-blocking, fast update
 *
 * Hardware (M5Stack Core2, GROVE Port A):
 *   MAX30100 VCC -> 3.3V
 *   MAX30100 GND -> GND
 *   MAX30100 SDA -> GPIO 32
 *   MAX30100 SCL -> GPIO 33
 *
 * Libraries:
 *   - M5Core2
 *   - MAX30100lib (oxullo) -> use MAX30100_PulseOximeter.h wrapper
 */

#include <M5Core2.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// -------- Sensor wrapper --------
PulseOximeter pox;

// -------- Readings / UI state --------
float heartRate = 0;
float spO2      = 0;
bool  sensorConnected = false;

unsigned long lastBeatMs            = 0;
unsigned long lastDisplayUpdateMs   = 0;
const unsigned long DISPLAY_MS      = 200;   // UI refresh rate (fast but not crazy)

// Non-blocking beat feedback (flash + vib)
bool          beatFlashActive = false;
unsigned long beatFlashStart  = 0;
const unsigned long BEAT_FLASH_MS = 80;

bool          vibActive = false;
unsigned long vibStart   = 0;
const unsigned long VIB_MS = 40; // short haptic pulse

// Graph memory (last 60 beats)
float heartRateHistory[60];
int   historyIndex = 0;
bool  historyFull  = false;

// Forward decl
void onBeatDetected();
void setupMainDisplay();
void updateDisplay();
void handleButtons();
void resetReadings();
void showHeartRateGraph();
void showSensorInfo();
void scanI2C();

// -------- Setup --------
void setup() {
  M5.begin(true, true, true, true);   // LCD, SD, Serial, I2C
  Serial.begin(115200);
  Serial.println("M5Stack Core2 + MAX30100");

  // Important: Core2 I2C pins are 32/33 on GROVE A
  Wire.begin(32, 33);

  // Basic UI
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(30, 10);
  M5.Lcd.println("Heart Rate Monitor");
  M5.Lcd.setCursor(30, 35);
  M5.Lcd.println("Init sensor...");

  // Init history
  for (int i = 0; i < 60; i++) heartRateHistory[i] = 0;

  // Start sensor
  // If your breakout lacks pull-ups or wiring is wrong, begin() will fail.
  if (!pox.begin()) {
    Serial.println("MAX30100 init FAILED");
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.setCursor(30, 60);
    M5.Lcd.println("Sensor Error!");
    sensorConnected = false;
  } else {
    Serial.println("MAX30100 init OK");
    sensorConnected = true;

    // Beat callback must be lightweight (no delays, no heavy I/O)
    pox.setOnBeatDetectedCallback(onBeatDetected);

    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.setCursor(30, 60);
    M5.Lcd.println("Sensor Ready");
  }

  setupMainDisplay();   // draw static UI
}

// -------- Main loop (keep it snappy!) --------
void loop() {
  M5.update();  // touch/buttons housekeeping

  if (sensorConnected) {
    // MUST be called very frequently (every few ms)
    pox.update();

    // Read current values (cheap)
    heartRate = pox.getHeartRate();
    spO2      = pox.getSpO2();

    // Handle beat flash + vibration without blocking
    const unsigned long now = millis();
    if (beatFlashActive && (now - beatFlashStart >= BEAT_FLASH_MS)) {
      // erase flash dot
      M5.Lcd.fillCircle(300, 30, 8, BLACK);
      beatFlashActive = false;
    }
    if (vibActive && (now - vibStart >= VIB_MS)) {
      // stop vibration
      M5.Axp.SetLDOEnable(3, false);
      vibActive = false;
    }

    // UI refresh at fixed cadence to reduce LCD overhead
    if (now - lastDisplayUpdateMs >= DISPLAY_MS) {
      updateDisplay();
      lastDisplayUpdateMs = now;
    }

    // Serial log at a human cadence
    static unsigned long lastPrint = 0;
    if (now - lastPrint >= 1500) {
      Serial.print("HR: ");
      Serial.print(heartRate, 1);
      Serial.print(" bpm  SpO2: ");
      Serial.print(spO2, 1);
      Serial.println(" %");
      lastPrint = now;
    }

    handleButtons();
  } else {
    // retry begin() occasionally without blocking everything
    static unsigned long lastRetry = 0;
    if (millis() - lastRetry >= 2000) {
      lastRetry = millis();
      if (pox.begin()) {
        sensorConnected = true;
        M5.Lcd.setTextColor(GREEN, BLACK);
        M5.Lcd.setCursor(160, 150);
        M5.Lcd.print("Reconnected ");
        // restore settings
        pox.setOnBeatDetectedCallback(onBeatDetected);
        pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);
      }
    }
  }

  // Tiny yield to WiFi/RTOS; do NOT use big delays
  delay(1);
}

// -------- Beat callback (keep it ultra-light) --------
void onBeatDetected() {
  lastBeatMs = millis();

  // Record history only if HR is valid
  const float hr = pox.getHeartRate();
  if (hr > 0.1f) {
    heartRateHistory[historyIndex] = hr;
    historyIndex = (historyIndex + 1) % 60;
    if (historyIndex == 0) historyFull = true;
  }

  // Schedule flash + vibration (done in loop, not here)
  beatFlashActive = true;
  beatFlashStart  = lastBeatMs;

  if (!vibActive) {
    vibActive = true;
    vibStart  = lastBeatMs;
    M5.Axp.SetLDOEnable(3, true); // start short buzz; loop() will stop it
  }

  // Draw the red dot quickly (fast I/O is OK; just no delays)
  M5.Lcd.fillCircle(300, 30, 8, RED);
}

// -------- UI helpers --------
void setupMainDisplay() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(96, 10);
  M5.Lcd.println("HR Monitor");

  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(20, 50);  M5.Lcd.println("Heart Rate:");
  M5.Lcd.setCursor(20, 100); M5.Lcd.println("SpO2:");
  M5.Lcd.setCursor(20, 150); M5.Lcd.println("Status:");

  // Touch “buttons”
  M5.Lcd.setTextSize(1);
  M5.Lcd.drawRect(5,   200, 100, 35, WHITE);  M5.Lcd.setCursor(35, 215);  M5.Lcd.print("Reset");
  M5.Lcd.drawRect(110, 200, 100, 35, WHITE);  M5.Lcd.setCursor(140, 215); M5.Lcd.print("Graph");
  M5.Lcd.drawRect(215, 200, 100, 35, WHITE);  M5.Lcd.setCursor(250, 215); M5.Lcd.print("Info");
}

void updateDisplay() {
  // Values region
  M5.Lcd.fillRect(160, 50, 150, 20, BLACK);
  M5.Lcd.fillRect(160, 100, 150, 20, BLACK);
  M5.Lcd.fillRect(160, 150, 150, 20, BLACK);

  // HR
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(160, 50);
  if (heartRate > 0.1f && heartRate < 255.0f) {
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.printf("%.1f BPM", heartRate);
  } else {
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.print("---");
  }

  // SpO2
  M5.Lcd.setCursor(160, 100);
  if (spO2 > 50.0f && spO2 <= 100.0f) {
    M5.Lcd.setTextColor(BLUE, BLACK);
    M5.Lcd.printf("%.1f %%", spO2);
  } else {
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.print("---");
  }

  // Status
  M5.Lcd.setCursor(160, 150);
  const unsigned long sinceBeat = millis() - lastBeatMs;
  if ((heartRate > 0.1f) && (sinceBeat < 3000)) {
    M5.Lcd.setTextColor(GREEN, BLACK); M5.Lcd.print("Active");
  } else if (sensorConnected) {
    M5.Lcd.setTextColor(YELLOW, BLACK); M5.Lcd.print("Waiting");
  } else {
    M5.Lcd.setTextColor(RED, BLACK); M5.Lcd.print("Error");
  }
  M5.Lcd.setTextColor(WHITE, BLACK);
}

void handleButtons() {
  // Physical BtnA/B/C wrappers (virtual on Core2)
  if (M5.BtnA.wasPressed()) resetReadings();
  if (M5.BtnB.wasPressed()) showHeartRateGraph();
  if (M5.BtnC.wasPressed()) showSensorInfo();

  // Touch regions
  auto &e = M5.Buttons.event;
  if (e & E_TOUCH) {
    auto pos = M5.Touch.getPressPoint();
    if (pos.y > 200) {
      if (pos.x < 107) resetReadings();
      else if (pos.x < 214) showHeartRateGraph();
      else showSensorInfo();
    }
  }
}

void resetReadings() {
  for (int i = 0; i < 60; i++) heartRateHistory[i] = 0;
  historyIndex = 0; historyFull = false;
  setupMainDisplay();
  M5.Lcd.setCursor(100, 190);
  M5.Lcd.setTextColor(GREEN, BLACK);
  M5.Lcd.print("Reset!");
}

void showHeartRateGraph() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.setCursor(100, 10);
  M5.Lcd.println("HR Graph");

  const int graphX = 20, graphY = 40, graphW = 280, graphH = 120;
  M5.Lcd.drawRect(graphX, graphY, graphW, graphH, WHITE);

  float minHR = 300, maxHR = 0;
  const int n = historyFull ? 60 : historyIndex;
  for (int i = 0; i < n; i++) {
    float v = heartRateHistory[i];
    if (v > 0.1f) { if (v < minHR) minHR = v; if (v > maxHR) maxHR = v; }
  }

  if (maxHR > minHR && n > 1) {
    for (int i = 1; i < n; i++) {
      float v1 = heartRateHistory[i-1], v2 = heartRateHistory[i];
      if (v1 > 0.1f && v2 > 0.1f) {
        int x1 = graphX + (i-1) * graphW / 60;
        int x2 = graphX + i     * graphW / 60;
        int y1 = graphY + graphH - (int)((v1 - minHR) * graphH / (maxHR - minHR));
        int y2 = graphY + graphH - (int)((v2 - minHR) * graphH / (maxHR - minHR));
        M5.Lcd.drawLine(x1, y1, x2, y2, RED);
      }
    }
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.setCursor(5, graphY + graphH - 10); M5.Lcd.print((int)minHR);
    M5.Lcd.setCursor(5, graphY + 5);           M5.Lcd.print((int)maxHR);
  } else {
    M5.Lcd.setTextColor(YELLOW, BLACK);
    M5.Lcd.setCursor(80, 100);
    M5.Lcd.println("No data to display");
  }

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.drawRect(110, 200, 100, 35, WHITE);
  M5.Lcd.setCursor(130, 215);
  M5.Lcd.println("Touch to return");

  // simple return on B or touch middle area
  while (!M5.BtnB.wasPressed()) {
    M5.update();
    auto &e = M5.Buttons.event;
    if (e & E_TOUCH) {
      auto pos = M5.Touch.getPressPoint();
      if (pos.y > 200 && pos.x >= 107 && pos.x < 214) break;
    }
    delay(10);
  }
  setupMainDisplay();
}

void showSensorInfo() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.setCursor(90, 10);
  M5.Lcd.println("Sensor Info");

  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.setCursor(20, 50);  M5.Lcd.println("Sensor: MAX30100");
  M5.Lcd.setCursor(20, 70);  M5.Lcd.println("Comm:   I2C");
  M5.Lcd.setCursor(20, 90);  M5.Lcd.println("Addr:   0x57");
  M5.Lcd.setCursor(20, 110); M5.Lcd.println("SDA:    GPIO 32");
  M5.Lcd.setCursor(20, 130); M5.Lcd.println("SCL:    GPIO 33");
  M5.Lcd.setCursor(20, 150); M5.Lcd.print("Status: ");
  if (sensorConnected) { M5.Lcd.setTextColor(GREEN, BLACK); M5.Lcd.println("Connected"); }
  else                 { M5.Lcd.setTextColor(RED,   BLACK); M5.Lcd.println("Disconnected"); }
  M5.Lcd.setTextColor(WHITE, BLACK);

  M5.Lcd.setCursor(20, 180); M5.Lcd.println("Place finger on sensor");
  M5.Lcd.setCursor(20, 195); M5.Lcd.println("covering LEDs + diode");

  M5.Lcd.drawRect(215, 200, 100, 35, WHITE);
  M5.Lcd.setCursor(230, 215);
  M5.Lcd.println("Touch to return");

  while (!M5.BtnC.wasPressed()) {
    M5.update();
    auto &e = M5.Buttons.event;
    if (e & E_TOUCH) {
      auto pos = M5.Touch.getPressPoint();
      if (pos.y > 200 && pos.x >= 214) break;
    }
    delay(10);
  }
  setupMainDisplay();
}

