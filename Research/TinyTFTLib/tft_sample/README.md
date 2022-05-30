# TFTサンプルアプリ

最新更新日：2022/5/30

## 概要

超小型TFTディスプレイ「KWH009ST01-F01」の動作確認用サンプルアプリについて解説しています。

<img src="../assets01/0002.jpg" width="300">

#### 使用機材
- PC（Arduinoをあらかじめインストールします）
- USBケーブル（Type-A to Type-A）
- Arduino UNO (R3)
- KWH009ST01-F01
- FPCコネクター（Arduino UNOとKWH009ST01-F01を接続するために必要）[注1]

[注1] FPCコネクターは、0.5mmピッチ8Pinタイプを使用します。

## TFTとの接続方法
TFT〜Arduino UNO間の配線は以下になります。

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


## プログラム
フォルダー`tft_sample`配下に格納しています。

#### ソースファイル

|#|モジュール名|説明|
|:---:|:---|:---|
|1|`tft_sample.ino`|メインモジュール|
|2|`TFT_ST7735.cpp`|Arduino TFT Library[注1]を１本にまとめたモジュール|

[注1]「KWH009ST01-F01」をはじめとする小型TFTディスプレイをArduinoに接続するためのライブラリー

### 関数群

TFTサンプルアプリ内で使用されている関数のみを掲載します。<br>
以下に分けて記述します。
- 上位関数
- サブ関数
- 低レベル関数

#### 上位関数
メインモジュールから呼び出されている関数になります。

|#|関数名|説明|
|:---:|:---|:---|
|1|`TFT_ST7735`|コンストラクター|
|2|`initR`|TFTに初期設定コマンドを投入|
|3|`setRotation`|TFT画面の向き（横長／縦長）を設定|
|4|`fillScreen`|TFT画面を指定の色で塗りつぶす|
|5|`setTextWrap`|文字列を右端で折り返すかどうかを設定|
|6|`setCursor`|文字列の先頭座標を設定|
|7|`setTextColor`|文字の色を指定|
|8|`setTextSize`|文字の大きさを指定|
|9|`print`|文字列を改行なしで表示|
|10|`println`|文字列を改行つきで表示|
|11|`invertDisplay`|TFT画面のピクセルをビット反転させる|

#### サブ関数
前述の関数群から、さらに呼び出されている関数になります。[注1]<br>
一部、説明はソースプログラムのコメントからそのまま抜粋しています。

|#|関数名|説明|
|:---:|:---|:---|
|1|`commonInit`|Initialization code common to all ST77XX displays|
|2|`begin`|Initialize ST77xx chip. <br>Connects to the ST77XX over SPI and sends initialization procedure commands|
|3|`displayInit`|Companion code to the initiliazation tables. <br>Reads and issues a series of LCD commands stored in PROGMEM byte array.|
|4|`sendCommand`|Send Command handles complete sending of commands and data|
|5|`fillRect`|Draw a filled rectangle to the display|
|6|`writeFillRectPreclipped`|A lower-level version of writeFillRect|
|7|`startWrite`|Call before issuing command(s) or data to display|
|8|`endWrite`|Call after issuing command(s) or data to display|
|9|`setAddrWindow`|SPI displays set an address window rectangle for blitting pixels|
|10|`writeCommand`|Write a single command byte to the display|
|11|`write`|Print one byte/character of data, used to support print()[注2]|
|12|`drawChar`|Draw a single character|
|13|`writePixel`|Draw a single pixel to the display at requested coordinates.|
|14|`writeFillRect`|Draw a filled rectangle to the display|
|15|`writeFastVLine`|Draw a vertical line on the display. <br>Performs edge clipping and rejection|

[注1] nRF5340に移植する場合でも、ハードウェア依存のコーディングは不要です。<br>
[注2] 前述の`print`、`println`関数を呼び出すと実行されます。

#### 低レベル関数
前述のサブ関数群から、さらに呼び出されている低レベルの関数になります。[注1]<br>
一部、説明はソースプログラムのコメントからそのまま抜粋しています。

|#|関数名|説明|
|:---:|:---|:---|
|1|`initSPI`|Configure microcontroller pins for TFT interfacing.|
|2|`SPI_CS_LOW`|Set the chip-select line LOW|
|3|`SPI_CS_HIGH`|Set the chip-select line HIGH|
|4|`SPI_DC_LOW`|Set the data/command line LOW (command mode)|
|5|`SPI_DC_HIGH`|Set the data/command line HIGH (data mode)|
|6|`SPI_BEGIN_TRANSACTION`|Start an SPI transaction if using the hardware SPI interface to the display|
|7|`SPI_END_TRANSACTION`|End an SPI transaction if using the hardware SPI interface to the display|
|8|`SPI_WRITE16`|Issue a single 16-bit value to the display|
|9|`SPI_WRITE32`|Issue a single 32-bit value to the display|
|10|`spiWrite`|Issue a single 8-bit value to the display|
|11|`writeColor`|Issue a series of pixels, all the same color|

[注1] nRF5340に移植する場合は、ハードウェア依存のコーディングが必要となります。

### 変数群

TFTサンプルアプリ内で使用されている変数のみを掲載します。<br>
以下に分けて記述します。
- クラス変数
- モジュール変数

#### クラス変数
一部、説明はソースプログラムのコメントからそのまま抜粋しています。

|#|関数名|初期値|説明|
|:---:|:---|:---|:---|
|1|`hwspi`|`SPI`[注1]|Hardware SPI values|
|2|`csPinMaskSet`|`digitalPinToBitMask(cs)`|Bitmask for chip select SET (OR)|
|3|`csPinMaskClr`|`~csPinMaskSet`|Bitmask for chip select CLEAR (AND)|
|4|`dcPinMaskSet`|`digitalPinToBitMask(dc)`|Bitmask for data/command SET (OR)|
|5|`dcPinMaskClr`|`~dcPinMaskSet`|Bitmask for data/command CLEAR (AND)|
|6|`connection`|`TFT_HARD_SPI`（`0`）||
|7|`_rst`|`TFT_RST`（`9`）|Reset pin #|
|8|`_cs`|`TFT_CS`（`10`）|Chip select pin #|
|9|`_dc`|`TFT_DC`（`8`）|Data/command pin #|
|10|`_xstart`|`0`|Internal framebuffer X offset|
|11|`_ystart`|`0`|Internal framebuffer Y offset|
|12|`invertOnCommand`|`ST77XX_INVON`（`0x21`）|Command to enable invert mode|
|13|`invertOffCommand`|`ST77XX_INVOFF`（`0x20`）|Command to disable invert mode|
|14|`WIDTH`|`ST7735_TFTWIDTH_80`（`80`）|This is the 'raw' display width - never changes|
|15|`HEIGHT`|`ST7735_TFTHEIGHT_160`（`160`）|This is the 'raw' display height - never changes|
|16|`_width`|`WIDTH`|Display width as modified by current rotation|
|17|`_height`|`HEIGHT`|Display height as modified by current rotation|
|18|`cursor_x`|`0`|x location to start print()ing text|
|19|`cursor_y`|`0`|y location to start print()ing text|
|20|`textcolor`|`0xFFFF`|16-bit background color for print()|
|21|`textbgcolor`|`0xFFFF`|16-bit text color for print()|
|22|`textsize_x`|`1`|Desired magnification in X-axis of text to print()|
|23|`textsize_y`|`1`|Desired magnification in Y-axis of text to print()|
|24|`rotation`|`0`|Display rotation (0 thru 3)|
|25|`wrap`|`true`|If set, 'wrap' text at right edge of display|
|26|`_cp437`|`false`|If set, use correct CP437 charset (default is off)|

[注1] メンバー`SPIClass *_spi`には、`SPI`というArduinoオブジェクトの参照が割り当てられています。

#### モジュール変数

|#|関数名|初期値|説明|
|:---:|:---|:---|:---|
|1|`font`|[注1]|Standard ASCII 5x7 font|

[注1] １文字あたり５バイト分のピクセルデータがプリセットされます。
