#include <Adafruit_GFX.h>
#include <SPI.h>

#include "TFT_ST7735.h"

#define TFT_CS        10
#define TFT_RST        9
#define TFT_DC         8

TFT_ST7735 tft = TFT_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Hello! ST7735 TFT Test"));

  // Init ST7735S mini display (0.96" 160x80 TFT)
  tft.initR();
  tft.setRotation(3);

  Serial.println(F("Initialized"));

  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  // tft print function!
  tftPrintTest();
  delay(4000);

  Serial.println("done");
  delay(1000);
}

void loop() {
  tft.invertDisplay(true);
  delay(2500);
  
  tft.invertDisplay(false);
  delay(2500);
}

void tftPrintTest() {
  tft.setTextWrap(false);

  tft.setCursor(0, 0);
  tft.setTextColor(swapbit(ST77XX_RED));
  tft.setTextSize(1);
  tft.println("Hello World!");
  
  delay(2000);

  tft.setTextColor(swapbit(ST77XX_YELLOW));
  tft.setTextSize(2);
  tft.println("Hello World!");
  
  delay(2000);
  
  tft.setTextColor(swapbit(ST77XX_GREEN));
  tft.setTextSize(3);
  tft.println("Hello World!");
  
  delay(2000);
  
  tft.setTextColor(swapbit(ST77XX_BLUE));
  tft.setTextSize(4);
  tft.println(123.45);

  delay(2000);
  
  tft.setCursor(0, 0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  
  tft.setTextSize(0);
  tft.println("Hello World!");

  float p = 3.1415926;

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  
  tft.setTextColor(ST77XX_MAGENTA);
  tft.print(millis() / 1000);
  
  tft.setTextColor(ST77XX_WHITE);
  tft.print(" seconds.");
}

uint16_t swapbit(uint16_t x) {
  uint16_t r = 0;
  uint8_t b = 16;
  while (b--) {
    r <<= 1;
    r |= (x & 1);
    x >>= 1;
  }
  return r;
}
