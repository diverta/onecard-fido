# TFTライブラリーの解析

最新更新日：2022/5/18

## 概要

超小型TFTディスプレイをArduino UNOに接続するためのライブラリーについて、解析を行います。

## 初期処理

`Adafruit_ST7735`というオブジェクトを初期化する部分になります。

```
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS        10
#define TFT_RST        9 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         8

// OPTION 1 (recommended) is to use the HARDWARE SPI pins, which are unique
// to each board and not reassignable. For Arduino Uno: MOSI = pin 11 and
// SCLK = pin 13. This is the fastest mode of operation and is required if
// using the breakout board's microSD card.

// For 1.44" and 1.8" TFT with ST7735 use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
```

#### ピン接続
前述のコード（デフォルト設定）の場合、TFT〜Arduino UNO間の配線は以下になります。

|TFT||Arduino UNO|説明|
|:--|:--:|:--|:--|
|#1（LEDA）|<--|3.3V|LEDバックライト電圧入力|
|#2（GND）|<--|GND|0V|
|#3（RESET）|<--|D9|TFTのリセット|
|#4（D/C）|<--|D8|データ／コマンド切替え|
|#5（SDA）|<--|D11（MOSI）|データ入力|
|#6（SCL）|<--|D13（SCLK）|クロック入力|
|#7（VDD）|<--|3.3V|動作電圧入力|
|#8（CS）|<--|D10（CS）|TFT通信開始|

#### オブジェクト生成

オブジェクト生成時の引数は、CS／DC／RESETピンを引数とします。

```
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
```

コードは以下のようになっています。
```
Adafruit_ST7735::Adafruit_ST7735(int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_ST77xx(ST7735_TFTWIDTH_128, ST7735_TFTHEIGHT_160, cs, dc, rst) {}
```

## setup関数

```
void setup(void) {
  Serial.begin(9600);
  Serial.print(F("Hello! ST77xx TFT Test"));

  // use this initializer (uncomment) if using a 0.96" 160x80 TFT:
  tft.initR(INITR_MINI160x80);  // Init ST7735S mini display

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
```

#### initR関数
`Adafruit-ST7735-Library/Adafruit_ST7735.cpp`内のコードです。<br>
前半では、コマンドセット`Rcmd1`、`Rcmd2green160x80`、`Rcmd3`を実行します（詳細は後述）。

```
#define INITR_MINI160x80 0x04
:
void Adafruit_ST7735::initR(uint8_t options) {
  commonInit(Rcmd1);
  if (options == INITR_GREENTAB) {
  :
  } else if (options == INITR_MINI160x80) {
    _height = ST7735_TFTWIDTH_80;
    _width = ST7735_TFTHEIGHT_160;
    displayInit(Rcmd2green160x80);
    _colstart = 24;
    _rowstart = 0;
  :
  }
  displayInit(Rcmd3);

  // Black tab, change MADCTL color filter
  if ((options == INITR_BLACKTAB) || (options == INITR_MINI160x80)) {
    uint8_t data = 0xC0;
    sendCommand(ST77XX_MADCTL, &data, 1);
  }

  if (options == INITR_HALLOWING) {
  :
  } else {
    tabcolor = options;
    setRotation(0);
  }
}
```

#### commonInit関数
`Adafruit-ST7735-Library/Adafruit_ST77xx.cpp`内のコードです。<br>
コマンドセット`Rcmd1`を実行します。

```
#define SPI_DEFAULT_FREQ 32000000 ///< Default SPI data clock frequency

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
:
protected:
  uint8_t _colstart = 0,   ///< Some displays need this changed to offset
      _rowstart = 0,       ///< Some displays need this changed to offset
      spiMode = SPI_MODE0; ///< Certain display needs MODE3 instead

void Adafruit_ST77xx::commonInit(const uint8_t *cmdList) {
  begin();

  if (cmdList) {
    displayInit(cmdList);
  }
}

void Adafruit_ST77xx::begin(uint32_t freq) {
  if (!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  _freq = freq;

  invertOnCommand = ST77XX_INVON;
  invertOffCommand = ST77XX_INVOFF;

  initSPI(freq, spiMode);
}

void Adafruit_ST77xx::displayInit(const uint8_t *addr) {

  uint8_t numCommands, cmd, numArgs;
  uint16_t ms;

  numCommands = pgm_read_byte(addr++); // Number of commands to follow
  while (numCommands--) {              // For each command...
    cmd = pgm_read_byte(addr++);       // Read command
    numArgs = pgm_read_byte(addr++);   // Number of args to follow
    ms = numArgs & ST_CMD_DELAY;       // If hibit set, delay follows args
    numArgs &= ~ST_CMD_DELAY;          // Mask out delay bit
    sendCommand(cmd, addr, numArgs);
    addr += numArgs;

    if (ms) {
      ms = pgm_read_byte(addr++); // Read post-command delay time (ms)
      if (ms == 255)
        ms = 500; // If 255, delay for 500 ms
      delay(ms);
    }
  }
}
```

`initSPI`関数は、`/Users/makmorit/Documents/GitHub/Adafruit-GFX-Library/Adafruit_SPITFT.cpp`に実装があります。<br>
`spiMode`は`SPI_MODE0`（アイドル時のクロックがLow、立ち上がりでサンプリング）とのことです。

`displayInit`の引数（アドレス）は、`PROGMEM`で定義した配列名（すなわちコマンドセット名称）になります。

#### Rcmd1
コマンドセット`Rcmd1`は、以下の部分になります。<br>
`Adafruit-ST7735-Library/Adafruit_ST7735.cpp`に記述されています。

<b>ご参考</b><br>
文中の`red or green tab`ですが、製品出荷時に貼付されている透明の赤いタブのことと思われます。<br>
本件調査に使用したFORMIKE社の「KWH009ST01-F01」には、赤いタブが貼付されておりました。

```
static const uint8_t PROGMEM
  :
  Rcmd1[] = {                       // 7735R init, part 1 (red or green tab)
    15,                             // 15 commands in list:
    ST77XX_SWRESET,   ST_CMD_DELAY, //  1: Software reset, 0 args, w/delay
      150,                          //     150 ms delay
    ST77XX_SLPOUT,    ST_CMD_DELAY, //  2: Out of sleep mode, 0 args, w/delay
      255,                          //     500 ms delay
    ST7735_FRMCTR1, 3,              //  3: Framerate ctrl - normal mode, 3 arg:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2, 3,              //  4: Framerate ctrl - idle mode, 3 args:
      0x01, 0x2C, 0x2D,             //     Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3, 6,              //  5: Framerate - partial mode, 6 args:
      0x01, 0x2C, 0x2D,             //     Dot inversion mode
      0x01, 0x2C, 0x2D,             //     Line inversion mode
    ST7735_INVCTR,  1,              //  6: Display inversion ctrl, 1 arg:
      0x07,                         //     No inversion
    ST7735_PWCTR1,  3,              //  7: Power control, 3 args, no delay:
      0xA2,
      0x02,                         //     -4.6V
      0x84,                         //     AUTO mode
    ST7735_PWCTR2,  1,              //  8: Power control, 1 arg, no delay:
      0xC5,                         //     VGH25=2.4C VGSEL=-10 VGH=3 * AVDD
    ST7735_PWCTR3,  2,              //  9: Power control, 2 args, no delay:
      0x0A,                         //     Opamp current small
      0x00,                         //     Boost frequency
    ST7735_PWCTR4,  2,              // 10: Power control, 2 args, no delay:
      0x8A,                         //     BCLK/2,
      0x2A,                         //     opamp current small & medium low
    ST7735_PWCTR5,  2,              // 11: Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1,  1,              // 12: Power control, 1 arg, no delay:
      0x0E,
    ST77XX_INVOFF,  0,              // 13: Don't invert display, no args
    ST77XX_MADCTL,  1,              // 14: Mem access ctl (directions), 1 arg:
      0xC8,                         //     row/col addr, bottom-top refresh
    ST77XX_COLMOD,  1,              // 15: set color mode, 1 arg, no delay:
      0x05 },                       //     16-bit color
    :
```
