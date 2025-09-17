/**
 * Based on the example by M5Stack & Example codes &  DUIET code by Rong-Hao Liang
 * ---------------------------------------------
 * M5STACK example: GettingWeight - Author: unknown
 * Example Code for Interactive Intelligent Products: Author: Rong-Hao Liang: r.liang@tue.nl
 * ---------------------------------------------
 * edittedd by hannah 20-8-2025 
 * selftracker: v3 
 */
//-----------------------
// This is for the m5core module (large screen)
// This code will display a green circle that changes in size based on the amount of weight detected. 
// the weight itself is shown in the circle. 
// This is a great way to test the scale and the steadyness of your enclosure
//------------------
#include <M5Stack.h>
#include <M5GFX.h>
#include "UNIT_SCALES.h"

M5GFX display;
M5Canvas canvas(&display);
UNIT_SCALES scales;

//--- stabilization variables-------------------------
float lastStableWeight;
float newStableWeight;
float lastDifference;
unsigned long stableStartTime = 0;
bool isStabilizing = false;
bool resetTriggered = false;

//---threshsolds----------------------------------------------
const float stable_threshold = 2.0;                     // grams
const unsigned long stable_time_threshold = 30000;      // ms (30 seconds)
const float difference_threshold = 20.0;                // grams


//_____________________________________________________________________________________________________________________________
void setup() {
    M5.begin();
    display.begin();
    canvas.setColorDepth(8);
    canvas.setFont(&fonts::efontCN_12);
    canvas.createSprite(display.width(), display.height());
    canvas.setTextSize(2);
// try to connect until success 
    while (!scales.begin(&Wire, 21, 22, DEVICE_DEFAULT_ADDR)) {
        Serial.println("scales connect error");
        M5.Lcd.print("scales connect error");//print on screen for easier debugging
        delay(1000);
    }
    scales.setLEDColor(0x001000);
    Serial.println("System ready.");
}

void loop() {
    float weight = scales.getWeight();
    int adc = scales.getRawADC();

    // --- stabilization check ---
    static float lastWeightReading = NAN;

    if (!isnan(lastWeightReading)) {
        if (fabs(weight - lastWeightReading) < stable_threshold) {
            if (!isStabilizing) {
                isStabilizing = true;
                stableStartTime = millis();
            } else if (millis() - stableStartTime >= stable_time_threshold) {
                // Stable for 30s
                if (isnan(lastStableWeight) || resetTriggered) {
                    lastStableWeight = weight;
                    resetTriggered = false;
                    lastDifference = NAN; // clear diff when resetting
                    Serial.printf("Stable weight saved: %.2f g\n", lastStableWeight);
                } else {
                    newStableWeight = weight;
                    if (fabs(newStableWeight - lastStableWeight) >= difference_threshold) {
                        lastDifference = newStableWeight - lastStableWeight;
                        Serial.printf("New stable weight: %.2f g\n", newStableWeight);
                        Serial.printf("Difference: %.2f g\n", lastDifference);
                        lastStableWeight = newStableWeight; // update baseline
                    }
                }
                isStabilizing = false; // reset state
            }
        } else {
            isStabilizing = false;
        }
    }
    lastWeightReading = weight;


//--------display circle on place of touch: 


// --- displa------- --> greeen circle in screeen
canvas.fillSprite(BLACK);

// map weight to circle radius
int maxRadius = min(display.width(), display.height()) / 2 - 10;
int radius = map((int)weight, 0, 500, 5, maxRadius); // adjust 500g to your max expected weight

// draw circle in the center
int centerX = display.width() / 2;
int centerY = display.height() / 2;
canvas.fillCircle(centerX, centerY, radius, GREEN);

// draw weight value in center
canvas.setTextSize(3);
canvas.setTextColor(BLACK); // contrast inside green
canvas.setTextDatum(middle_center);
canvas.drawString(String(weight, 1) + "g", centerX, centerY);

// push to screen
canvas.pushSprite(0, 0);




//----old ui-----
    // // --- display UI ---
    // canvas.fillSprite(BLACK);
    // canvas.setTextSize(2);
    // canvas.drawString("Unit Scale Weight Getting", 10, 10);

    // // live weight
    // canvas.setTextColor(WHITE);
    // canvas.setCursor(10, 50);
    // canvas.printf("WEIGHT:");
    // canvas.setTextColor(GREEN);
    // canvas.setTextSize(3);
    // canvas.printf("%.2fg", weight);

    // // saved stable weight
    // canvas.setTextColor(WHITE);
    // canvas.setTextSize(2);
    // canvas.setCursor(10, 160);
    // canvas.printf("Stable:");
    // if (!isnan(lastStableWeight)) {
    //     canvas.setTextColor(CYAN);
    //     canvas.setTextSize(3);
    //     canvas.printf(" %.2fg", lastStableWeight);
    // }

    // // difference
    // canvas.setCursor(10, 210);
    // canvas.setTextColor(WHITE);
    // canvas.setTextSize(2);
    // canvas.printf("Diff:");
    // if (!isnan(lastDifference)) {
    //     canvas.setTextColor(ORANGE);
    //     canvas.setTextSize(3);
    //     canvas.printf(" %.2fg", lastDifference);
    // }
    // canvas.pushSprite(0, 0);

    // --- button signals ---
    M5.update();
    if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed()) {
        resetTriggered = true;
        Serial.println("Reset triggered by signal A/B.");
    }
}
