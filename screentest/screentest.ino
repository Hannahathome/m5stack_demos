#include <M5Core2.h>

TFT_eSprite spr = TFT_eSprite(&M5.Lcd);

int backlight = 2800;

void setup()
{
  M5.begin();
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(40, 0);
  M5.Lcd.println("Screentest");

  // Create a smaller sprite 
  spr.createSprite(200, 100);  // Smaller, easier to see
  spr.fillSprite(TFT_RED);     // Bright red background
}

void loop()
{
  M5.update();
  backlight = backlight + 1;
  if (backlight >= 3300)
    backlight = 0;
  // M5.Axp.SetLcdVoltage(backlight);
  
  // Clear the sprite with bright red
  spr.fillSprite(TFT_RED);
  spr.setTextDatum(4);  // Center alignment
  spr.setTextSize(2);
  spr.setTextColor(TFT_WHITE, TFT_RED);  // White text on red background
  spr.drawString(String(backlight), 100, 50);  // Center of the 200x100 sprite
  
  // Push sprite at a visible location
  spr.pushSprite(60, 80);
  delay(100);  // Slower update to see changes
}