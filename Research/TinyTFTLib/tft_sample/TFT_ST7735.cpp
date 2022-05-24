#include "TFT_ST7735.h"
#include <limits.h>
#ifndef ARDUINO_STM32_FEATHER
#include "pins_arduino.h"
#include "wiring_private.h"
#endif
#include <SPI.h>

#define SPI_DEFAULT_FREQ 32000000 ///< Default SPI data clock frequency

/*!
    @brief  Instantiate ST7735 driver with default hardware SPI
    @param  w    Display width in pixels at default rotation setting (0)
    @param  h    Display height in pixels at default rotation setting (0)
    @param  cs   Chip select pin #
    @param  dc   Data/Command pin #
    @param  rst  Reset pin # (optional, pass -1 if unused)
*/
TFT_ST7735::TFT_ST7735(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_SPITFT(w, h, cs, dc, rst) {}

/*!
    @brief  Companion code to the initiliazation tables. Reads and issues
            a series of LCD commands stored in PROGMEM byte array.
    @param  addr  Flash memory array with commands and data to send
*/
void TFT_ST7735::displayInit(const uint8_t *addr) {

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

/*!
    @brief  Initialize ST77xx chip. Connects to the ST77XX over SPI and
            sends initialization procedure commands
    @param  freq  Desired SPI clock frequency
*/
void TFT_ST7735::begin(uint32_t freq) {
  if (!freq) {
    freq = SPI_DEFAULT_FREQ;
  }
  _freq = freq;

  invertOnCommand = ST77XX_INVON;
  invertOffCommand = ST77XX_INVOFF;

  initSPI(freq, spiMode);
}

/*!
    @brief  Initialization code common to all ST77XX displays
    @param  cmdList  Flash memory array with commands and data to send
*/
void TFT_ST7735::commonInit(const uint8_t *cmdList) {
  begin();

  if (cmdList) {
    displayInit(cmdList);
  }
}

/*!
  @brief  SPI displays set an address window rectangle for blitting pixels
  @param  x  Top left corner x coordinate
  @param  y  Top left corner x coordinate
  @param  w  Width of window
  @param  h  Height of window
*/
void TFT_ST7735::setAddrWindow(uint16_t x, uint16_t y, uint16_t w,
                                    uint16_t h) {
  x += _xstart;
  y += _ystart;
  uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
  uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

  writeCommand(ST77XX_CASET); // Column addr set
  SPI_WRITE32(xa);

  writeCommand(ST77XX_RASET); // Row addr set
  SPI_WRITE32(ya);

  writeCommand(ST77XX_RAMWR); // write to RAM
}

/*!
    @brief  Set origin of (0,0) of display with offsets
    @param  col  The offset from 0 for the column address
    @param  row  The offset from 0 for the row address
*/
void TFT_ST7735::setColRowStart(int8_t col, int8_t row) {
  _colstart = col;
  _rowstart = row;
}

/*!
 @brief  Change whether display is on or off
 @param  enable True if you want the display ON, false OFF
 */
void TFT_ST7735::enableDisplay(boolean enable) {
  sendCommand(enable ? ST77XX_DISPON : ST77XX_DISPOFF);
}

/*!
 @brief  Change whether TE pin output is on or off
 @param  enable True if you want the TE pin ON, false OFF
 */
void TFT_ST7735::enableTearing(boolean enable) {
  sendCommand(enable ? ST77XX_TEON : ST77XX_TEOFF);
}

/*!
 @brief  Change whether sleep mode is on or off
 @param  enable True if you want sleep mode ON, false OFF
 */
void TFT_ST7735::enableSleep(boolean enable) {
  sendCommand(enable ? ST77XX_SLPIN : ST77XX_SLPOUT);
}

// SCREEN INITIALIZATION ***************************************************
// Rather than a bazillion writecommand() and writedata() calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.

// clang-format off
static const uint8_t PROGMEM
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

  Rcmd2green160x80[] = {            // 7735R init, part 2 (mini 160x80)
    2,                              //  2 commands in list:
    ST77XX_CASET,   4,              //  1: Column addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x4F,                   //     XEND = 79
    ST77XX_RASET,   4,              //  2: Row addr set, 4 args, no delay:
      0x00, 0x00,                   //     XSTART = 0
      0x00, 0x9F },                 //     XEND = 159

  Rcmd3[] = {                       // 7735R init, part 3 (red or green tab)
    4,                              //  4 commands in list:
    ST7735_GMCTRP1, 16      ,       //  1: Gamma Adjustments (pos. polarity), 16 args + delay:
      0x02, 0x1c, 0x07, 0x12,       //     (Not entirely necessary, but provides
      0x37, 0x32, 0x29, 0x2d,       //      accurate colors)
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1, 16      ,       //  2: Gamma Adjustments (neg. polarity), 16 args + delay:
      0x03, 0x1d, 0x07, 0x06,       //     (Not entirely necessary, but provides
      0x2E, 0x2C, 0x29, 0x2D,       //      accurate colors)
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST77XX_NORON,     ST_CMD_DELAY, //  3: Normal display on, no args, w/delay
      10,                           //     10 ms delay
    ST77XX_DISPON,    ST_CMD_DELAY, //  4: Main screen turn on, no args w/delay
      100 };                        //     100 ms delay

// clang-format on

/*!
    @brief  Initialization code common to all ST7735R displays
*/
void TFT_ST7735::initR(void) {
  commonInit(Rcmd1);

  _height = ST7735_TFTWIDTH_80;
  _width = ST7735_TFTHEIGHT_160;
  displayInit(Rcmd2green160x80);
  _colstart = 24;
  _rowstart = 0;

  displayInit(Rcmd3);

  // Change MADCTL color filter
  uint8_t data = 0xC0;
  sendCommand(ST77XX_MADCTL, &data, 1);

  setRotation(0);
}

/*!
    @brief  Set origin of (0,0) and orientation of TFT display
    @param  m  The index for rotation, from 0-3 inclusive
*/
void TFT_ST7735::setRotation(uint8_t m) {
  uint8_t madctl = 0;

  // can't be higher than 3
  rotation = m & 3;

  switch (rotation) {
  case 0:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;
    
    _height = ST7735_TFTHEIGHT_160;
    _width = ST7735_TFTWIDTH_80;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;

  case 1:
    madctl = ST77XX_MADCTL_MY | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;

    _width = ST7735_TFTHEIGHT_160;
    _height = ST7735_TFTWIDTH_80;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
    
  case 2:
    madctl = ST77XX_MADCTL_RGB;

    _height = ST7735_TFTHEIGHT_160;
    _width = ST7735_TFTWIDTH_80;
    _xstart = _colstart;
    _ystart = _rowstart;
    break;
    
  case 3:
    madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MV | ST77XX_MADCTL_RGB;

    _width = ST7735_TFTHEIGHT_160;
    _height = ST7735_TFTWIDTH_80;
    _ystart = _colstart;
    _xstart = _rowstart;
    break;
  }

  sendCommand(ST77XX_MADCTL, &madctl, 1);
}
