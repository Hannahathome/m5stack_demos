// tell me your address code, made to ensure you always know the address of your i2c devices 
// this code is for m5core2
// this code was co-created with chatgpt5 for clarity 

#include <M5Stack.h>
#include <Wire.h>

void setup() {
  M5.begin();
  Wire.begin();            // Use default SDA=21, SCL=22
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int nDevices = 0;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);
      Serial.println(" !");
      nDevices++;
    }
  }

  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  } else {
    Serial.println("Done\n");
  }

  delay(2000); // scan every 2 sec
}
