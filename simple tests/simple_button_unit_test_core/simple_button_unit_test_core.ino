// connect unit button using jumpwires to the CORE (1) 
// red to 5v
// black to ground
// yellow no connection 
// white to 26 (or any of the others mentionned in the code)

#include <M5Stack.h>
#define BUTTON_PIN 26  // pick GPIO 32, 33, 25, 26, 36, or 39

void setup() {
    M5.begin();
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP); // active-low button
    M5.Lcd.println("Core basic button test");
}

void loop() {
    int state = digitalRead(BUTTON_PIN);
    Serial.println(state == LOW ? "PRESSED" : "RELEASED");

    M5.Lcd.setCursor(0, 40);
    M5.Lcd.print("the button is: ");
    M5.Lcd.println(state == LOW ? "PRESSED " : "RELEASED");

    delay(100);
}
