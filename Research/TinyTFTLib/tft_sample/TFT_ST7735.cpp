#include "TFT_ST7735.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"

#define SPI_DEFAULT_FREQ 32000000 ///< Default SPI data clock frequency
#define AVR_WRITESPI(x) for (SPDR = (x); (!(SPSR & _BV(SPIF)));)

// Possible values for TFT_SPITFT.connection:
// Display interface = hardware SPI
#define TFT_HARD_SPI 0 

/*!
    @brief  Instantiate ST7735 driver with default hardware SPI
    @param  w    Display width in pixels at default rotation setting (0)
    @param  h    Display height in pixels at default rotation setting (0)
    @param  cs   Chip select pin #
    @param  dc   Data/Command pin #
    @param  rst  Reset pin # (optional, pass -1 if unused)
*/
TFT_ST7735::TFT_ST7735(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t rst)
    : Adafruit_GFX(w, h), connection(TFT_HARD_SPI), _rst(rst), _cs(cs), _dc(dc) {
  // This just invokes the hardware SPI constructor below,
  // passing the default SPI device (&SPI).
  hwspi._spi = &SPI;
  dcPort = (PORTreg_t)portOutputRegister(digitalPinToPort(dc));
  dcPinMaskSet = digitalPinToBitMask(dc);
  if (cs >= 0) {
    csPort = (PORTreg_t)portOutputRegister(digitalPinToPort(cs));
    csPinMaskSet = digitalPinToBitMask(cs);
  } else {
    // No chip-select line defined; might be permanently tied to GND.
    // Assign a valid GPIO register (though not used for CS), and an
    // empty pin bitmask...the nonsense bit-twiddling might be faster
    // than checking _cs and possibly branching.
    csPort = dcPort;
    csPinMaskSet = 0;
  }
  csPinMaskClr = ~csPinMaskSet;
  dcPinMaskClr = ~dcPinMaskSet;
}

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

/*!
    @brief  Configure microcontroller pins for TFT interfacing. Typically
            called by a subclass' begin() function.
    @param  freq     SPI frequency when using hardware SPI. If default (0)
                     is passed, will fall back on a device-specific value.
                     Value is ignored when using software SPI or parallel
                     connection.
    @param  spiMode  SPI mode when using hardware SPI. MUST be one of the
                     values SPI_MODE0, SPI_MODE1, SPI_MODE2 or SPI_MODE3
                     defined in SPI.h. Do NOT attempt to pass '0' for
                     SPI_MODE0 and so forth...the values are NOT the same!
                     Use ONLY the defines! (Pity it's not an enum.)
    @note   Another anachronistically-named function; this is called even
            when the display connection is parallel (not SPI). Also, this
            could probably be made private...quite a few class functions
            were generously put in the public section.
*/
void TFT_ST7735::initSPI(uint32_t freq, uint8_t spiMode) {

  if (!freq)
    freq = DEFAULT_SPI_FREQ; // If no freq specified, use default

  // Init basic control pins common to all connection types
  if (_cs >= 0) {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH); // Deselect
  }
  pinMode(_dc, OUTPUT);
  digitalWrite(_dc, HIGH); // Data mode

  if (connection == TFT_HARD_SPI) {

    hwspi.settings = SPISettings(freq, MSBFIRST, spiMode);
    hwspi._mode = spiMode; // Save spiMode value for later
    // Call hwspi._spi->begin() ONLY if this is among the 'established'
    // SPI interfaces in variant.h. For DIY roll-your-own SERCOM SPIs,
    // begin() and pinPeripheral() calls MUST be made in one's calling
    // code, BEFORE the screen-specific begin/init function is called.
    // Reason for this is that SPI::begin() makes its own calls to
    // pinPeripheral() based on g_APinDescription[n].ulPinType, which
    // on non-established SPI interface pins will always be PIO_DIGITAL
    // or similar, while we need PIO_SERCOM or PIO_SERCOM_ALT...it's
    // highly unique between devices and variants for each pin or
    // SERCOM so we can't make those calls ourselves here. And the SPI
    // device needs to be set up before calling this because it's
    // immediately followed with initialization commands. Blargh.
    if (hwspi._spi == &SPI) {
      hwspi._spi->begin();
    }
  }

  if (_rst >= 0) {
    // Toggle _rst low to reset
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, HIGH);
    delay(100);
    digitalWrite(_rst, LOW);
    delay(100);
    digitalWrite(_rst, HIGH);
    delay(200);
  }
}

/*!
    @brief  Allow changing the SPI clock speed after initialization
    @param  freq Desired frequency of SPI clock, may not be the
    end frequency you get based on what the chip can do!
*/
void TFT_ST7735::setSPISpeed(uint32_t freq) {
  hwspi.settings = SPISettings(freq, MSBFIRST, hwspi._mode);
}

/*!
    @brief  Call before issuing command(s) or data to display. Performs
            chip-select (if required) and starts an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
void TFT_ST7735::startWrite(void) {
  SPI_BEGIN_TRANSACTION();
  if (_cs >= 0)
    SPI_CS_LOW();
}

/*!
    @brief  Call after issuing command(s) or data to display. Performs
            chip-deselect (if required) and ends an SPI transaction (if
            using hardware SPI and transactions are supported). Required
            for all display types; not an SPI-specific function.
*/
void TFT_ST7735::endWrite(void) {
  if (_cs >= 0)
    SPI_CS_HIGH();
  SPI_END_TRANSACTION();
}

// -------------------------------------------------------------------------
// Lower-level graphics operations. These functions require a chip-select
// and/or SPI transaction around them (via startWrite(), endWrite() above).
// Higher-level graphics primitives might start a single transaction and
// then make multiple calls to these functions (e.g. circle or text
// rendering might make repeated lines or rects) before ending the
// transaction. It's more efficient than starting a transaction every time.

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Not self-contained; should follow a startWrite() call.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void TFT_ST7735::writePixel(int16_t x, int16_t y, uint16_t color) {
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
  }
}

/*!
    @brief  Swap bytes in an array of pixels; converts little-to-big or
            big-to-little endian. Used by writePixels() below in some
            situations, but may also be helpful for user code occasionally.
    @param  src   Source address of 16-bit pixels buffer.
    @param  len   Number of pixels to byte-swap.
    @param  dest  Optional destination address if different than src --
                  otherwise, if NULL (default) or same address is passed,
                  pixel buffer is overwritten in-place.
*/
void TFT_ST7735::swapBytes(uint16_t *src, uint32_t len, uint16_t *dest) {
  if (!dest)
    dest = src; // NULL -> overwrite src buffer
  for (uint32_t i = 0; i < len; i++) {
    dest[i] = __builtin_bswap16(src[i]);
  }
}

/*!
    @brief  Issue a series of pixels from memory to the display. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  colors     Pointer to array of 16-bit pixel values in '565' RGB
                       format.
    @param  len        Number of elements in 'colors' array.
    @param  block      If true (default case if unspecified), function blocks
                       until DMA transfer is complete. This is simply IGNORED
                       if DMA is not enabled. If false, the function returns
                       immediately after the last DMA transfer is started,
                       and one should use the dmaWait() function before
                       doing ANY other display-related activities (or even
                       any SPI-related activities, if using an SPI display
                       that shares the bus with other devices).
    @param  bigEndian  If true, bitmap in memory is in big-endian order (most
                       significant byte first). By default this is false, as
                       most microcontrollers seem to be little-endian and
                       16-bit pixel values must be byte-swapped before
                       issuing to the display (which tend toward big-endian
                       when using SPI or 8-bit parallel). If an application
                       can optimize around this -- for example, a bitmap in a
                       uint16_t array having the byte values already ordered
                       big-endian, this can save time here, ESPECIALLY if
                       using this function's non-blocking DMA mode.
*/
void TFT_ST7735::writePixels(uint16_t *colors, uint32_t len, bool block,
                                  bool bigEndian) {

  if (!len)
    return; // Avoid 0-byte transfers

  // avoid paramater-not-used complaints
  (void)block;
  (void)bigEndian;

  // All other cases (bitbang SPI or non-DMA hard SPI or parallel),
  // use a loop with the normal 16-bit data write function:

  if (!bigEndian) {
    while (len--) {
      SPI_WRITE16(*colors++);
    }
  } else {
    // Well this is awkward. SPI_WRITE16() was designed for little-endian
    // hosts and big-endian displays as that's nearly always the typical
    // case. If the bigEndian flag was set, data is already in display's
    // order...so each pixel needs byte-swapping before being issued.
    // Rather than having a separate big-endian SPI_WRITE16 (adding more
    // bloat), it's preferred if calling function is smart and only uses
    // bigEndian where DMA is supported. But we gotta handle this...
    while (len--) {
      SPI_WRITE16(__builtin_bswap16(*colors++));
    }
  }
}

/*!
    @brief  Wait for the last DMA transfer in a prior non-blocking
            writePixels() call to complete. This does nothing if DMA
            is not enabled, and is not needed if blocking writePixels()
            was used (as is the default case).
*/
void TFT_ST7735::dmaWait(void) {
}

/*!
    @brief  Check if DMA transfer is active. Always returts false if DMA
            is not enabled.
    @return true if DMA is enabled and transmitting data, false otherwise.
*/
bool TFT_ST7735::dmaBusy(void) const {
  return false;
}

/*!
    @brief  Issue a series of pixels, all the same color. Not self-
            contained; should follow startWrite() and setAddrWindow() calls.
    @param  color  16-bit pixel color in '565' RGB format.
    @param  len    Number of pixels to draw.
*/
void TFT_ST7735::writeColor(uint16_t color, uint32_t len) {

  if (!len)
    return; // Avoid 0-byte transfers

  uint8_t hi = color >> 8, lo = color;

  // All other cases (non-DMA hard SPI, bitbang SPI, parallel)...

  if (connection == TFT_HARD_SPI) {
    while (len--) {
  #if defined(__AVR__)
        AVR_WRITESPI(hi);
        AVR_WRITESPI(lo);
  #elif defined(ESP32)
        hwspi._spi->write(hi);
        hwspi._spi->write(lo);
  #else
        hwspi._spi->transfer(hi);
        hwspi._spi->transfer(lo);
  #endif
    }
  }
}

/*!
    @brief  Draw a filled rectangle to the display. Not self-contained;
            should follow startWrite(). Typically used by higher-level
            graphics primitives; user code shouldn't need to call this and
            is likely to use the self-contained fillRect() instead.
            writeFillRect() performs its own edge clipping and rejection;
            see writeFillRectPreclipped() for a more 'raw' implementation.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   Written in this deep-nested way because C by definition will
            optimize for the 'if' case, not the 'else' -- avoids branches
            and rejects clipped rectangles at the least-work possibility.
*/
void TFT_ST7735::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                                    uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            writeFillRectPreclipped(x, y, w, h, color);
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Performs edge clipping
            and rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastHLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline TFT_ST7735::writeFastHLine(int16_t x, int16_t y, int16_t w,
                                            uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        writeFillRectPreclipped(x, y, w, 1, color);
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Performs edge clipping and
            rejection. Not self-contained; should follow startWrite().
            Typically used by higher-level graphics primitives; user code
            shouldn't need to call this and is likely to use the self-
            contained drawFastVLine() instead.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
*/
void inline TFT_ST7735::writeFastVLine(int16_t x, int16_t y, int16_t h,
                                            uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        writeFillRectPreclipped(x, y, 1, h, color);
      }
    }
  }
}

/*!
    @brief  A lower-level version of writeFillRect(). This version requires
            all inputs are in-bounds, that width and height are positive,
            and no part extends offscreen. NO EDGE CLIPPING OR REJECTION IS
            PERFORMED. If higher-level graphics primitives are written to
            handle their own clipping earlier in the drawing process, this
            can avoid unnecessary function calls and repeated clipping
            operations in the lower-level functions.
    @param  x      Horizontal position of first corner. MUST BE WITHIN
                   SCREEN BOUNDS.
    @param  y      Vertical position of first corner. MUST BE WITHIN SCREEN
                   BOUNDS.
    @param  w      Rectangle width in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  h      Rectangle height in pixels. MUST BE POSITIVE AND NOT
                   EXTEND OFF SCREEN.
    @param  color  16-bit fill color in '565' RGB format.
    @note   This is a new function, no graphics primitives besides rects
            and horizontal/vertical lines are written to best use this yet.
*/
inline void TFT_ST7735::writeFillRectPreclipped(int16_t x, int16_t y,
                                                     int16_t w, int16_t h,
                                                     uint16_t color) {
  setAddrWindow(x, y, w, h);
  writeColor(color, (uint32_t)w * h);
}

// -------------------------------------------------------------------------
// Ever-so-slightly higher-level graphics operations. Similar to the 'write'
// functions above, but these contain their own chip-select and SPI
// transactions as needed (via startWrite(), endWrite()). They're typically
// used solo -- as graphics primitives in themselves, not invoked by higher-
// level primitives (which should use the functions above for better
// performance).

/*!
    @brief  Draw a single pixel to the display at requested coordinates.
            Self-contained and provides its own transaction as needed
            (see writePixel(x,y,color) for a lower-level variant).
            Edge clipping is performed here.
    @param  x      Horizontal position (0 = left).
    @param  y      Vertical position   (0 = top).
    @param  color  16-bit pixel color in '565' RGB format.
*/
void TFT_ST7735::drawPixel(int16_t x, int16_t y, uint16_t color) {
  // Clip first...
  if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
    // THEN set up transaction (if needed) and draw...
    startWrite();
    setAddrWindow(x, y, 1, 1);
    SPI_WRITE16(color);
    endWrite();
  }
}

/*!
    @brief  Draw a filled rectangle to the display. Self-contained and
            provides its own transaction as needed (see writeFillRect() or
            writeFillRectPreclipped() for lower-level variants). Edge
            clipping and rejection is performed here.
    @param  x      Horizontal position of first corner.
    @param  y      Vertical position of first corner.
    @param  w      Rectangle width in pixels (positive = right of first
                   corner, negative = left of first corner).
    @param  h      Rectangle height in pixels (positive = below first
                   corner, negative = above first corner).
    @param  color  16-bit fill color in '565' RGB format.
    @note   This repeats the writeFillRect() function almost in its entirety,
            with the addition of a transaction start/end. It's done this way
            (rather than starting the transaction and calling writeFillRect()
            to handle clipping and so forth) so that the transaction isn't
            performed at all if the rectangle is rejected. It's really not
            that much code.
*/
void TFT_ST7735::fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                               uint16_t color) {
  if (w && h) {   // Nonzero width and height?
    if (w < 0) {  // If negative width...
      x += w + 1; //   Move X to left edge
      w = -w;     //   Use positive width
    }
    if (x < _width) { // Not off right
      if (h < 0) {    // If negative height...
        y += h + 1;   //   Move Y to top edge
        h = -h;       //   Use positive height
      }
      if (y < _height) { // Not off bottom
        int16_t x2 = x + w - 1;
        if (x2 >= 0) { // Not off left
          int16_t y2 = y + h - 1;
          if (y2 >= 0) { // Not off top
            // Rectangle partly or fully overlaps screen
            if (x < 0) {
              x = 0;
              w = x2 + 1;
            } // Clip left
            if (y < 0) {
              y = 0;
              h = y2 + 1;
            } // Clip top
            if (x2 >= _width) {
              w = _width - x;
            } // Clip right
            if (y2 >= _height) {
              h = _height - y;
            } // Clip bottom
            startWrite();
            writeFillRectPreclipped(x, y, w, h, color);
            endWrite();
          }
        }
      }
    }
  }
}

/*!
    @brief  Draw a horizontal line on the display. Self-contained and
            provides its own transaction as needed (see writeFastHLine() for
            a lower-level variant). Edge clipping and rejection is performed
            here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  w      Line width in pixels (positive = right of first point,
                   negative = point of first corner).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastHLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastHLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void TFT_ST7735::drawFastHLine(int16_t x, int16_t y, int16_t w,
                                    uint16_t color) {
  if ((y >= 0) && (y < _height) && w) { // Y on screen, nonzero width
    if (w < 0) {                        // If negative width...
      x += w + 1;                       //   Move X to left edge
      w = -w;                           //   Use positive width
    }
    if (x < _width) { // Not off right
      int16_t x2 = x + w - 1;
      if (x2 >= 0) { // Not off left
        // Line partly or fully overlaps screen
        if (x < 0) {
          x = 0;
          w = x2 + 1;
        } // Clip left
        if (x2 >= _width) {
          w = _width - x;
        } // Clip right
        startWrite();
        writeFillRectPreclipped(x, y, w, 1, color);
        endWrite();
      }
    }
  }
}

/*!
    @brief  Draw a vertical line on the display. Self-contained and provides
            its own transaction as needed (see writeFastHLine() for a lower-
            level variant). Edge clipping and rejection is performed here.
    @param  x      Horizontal position of first point.
    @param  y      Vertical position of first point.
    @param  h      Line height in pixels (positive = below first point,
                   negative = above first point).
    @param  color  16-bit line color in '565' RGB format.
    @note   This repeats the writeFastVLine() function almost in its
            entirety, with the addition of a transaction start/end. It's
            done this way (rather than starting the transaction and calling
            writeFastVLine() to handle clipping and so forth) so that the
            transaction isn't performed at all if the line is rejected.
*/
void TFT_ST7735::drawFastVLine(int16_t x, int16_t y, int16_t h,
                                    uint16_t color) {
  if ((x >= 0) && (x < _width) && h) { // X on screen, nonzero height
    if (h < 0) {                       // If negative height...
      y += h + 1;                      //   Move Y to top edge
      h = -h;                          //   Use positive height
    }
    if (y < _height) { // Not off bottom
      int16_t y2 = y + h - 1;
      if (y2 >= 0) { // Not off top
        // Line partly or fully overlaps screen
        if (y < 0) {
          y = 0;
          h = y2 + 1;
        } // Clip top
        if (y2 >= _height) {
          h = _height - y;
        } // Clip bottom
        startWrite();
        writeFillRectPreclipped(x, y, 1, h, color);
        endWrite();
      }
    }
  }
}

/*!
    @brief  Essentially writePixel() with a transaction around it. I don't
            think this is in use by any of our code anymore (believe it was
            for some older BMP-reading examples), but is kept here in case
            any user code relies on it. Consider it DEPRECATED.
    @param  color  16-bit pixel color in '565' RGB format.
*/
void TFT_ST7735::pushColor(uint16_t color) {
  startWrite();
  SPI_WRITE16(color);
  endWrite();
}

/*!
    @brief  Draw a 16-bit image (565 RGB) at the specified (x,y) position.
            For 16-bit display devices; no color reduction performed.
            Adapted from https://github.com/PaulStoffregen/ILI9341_t3
            by Marc MERLIN. See examples/pictureEmbed to use this.
            5/6/2017: function name and arguments have changed for
            compatibility with current GFX library and to avoid naming
            problems in prior implementation.  Formerly drawBitmap() with
            arguments in different order. Handles its own transaction and
            edge clipping/rejection.
    @param  x        Top left corner horizontal coordinate.
    @param  y        Top left corner vertical coordinate.
    @param  pcolors  Pointer to 16-bit array of pixel values.
    @param  w        Width of bitmap in pixels.
    @param  h        Height of bitmap in pixels.
*/
void TFT_ST7735::drawRGBBitmap(int16_t x, int16_t y, uint16_t *pcolors,
                                    int16_t w, int16_t h) {

  int16_t x2, y2;                 // Lower-right coord
  if ((x >= _width) ||            // Off-edge right
      (y >= _height) ||           // " top
      ((x2 = (x + w - 1)) < 0) || // " left
      ((y2 = (y + h - 1)) < 0))
    return; // " bottom

  int16_t bx1 = 0, by1 = 0, // Clipped top-left within bitmap
      saveW = w;            // Save original bitmap width value
  if (x < 0) {              // Clip left
    w += x;
    bx1 = -x;
    x = 0;
  }
  if (y < 0) { // Clip top
    h += y;
    by1 = -y;
    y = 0;
  }
  if (x2 >= _width)
    w = _width - x; // Clip right
  if (y2 >= _height)
    h = _height - y; // Clip bottom

  pcolors += by1 * saveW + bx1; // Offset bitmap ptr to clipped top-left
  startWrite();
  setAddrWindow(x, y, w, h); // Clipped area
  while (h--) {              // For each (clipped) scanline...
    writePixels(pcolors, w); // Push one (clipped) row
    pcolors += saveW;        // Advance pointer by one full (unclipped) line
  }
  endWrite();
}

// -------------------------------------------------------------------------
// Miscellaneous class member functions that don't draw anything.

/*!
    @brief  Invert the colors of the display (if supported by hardware).
            Self-contained, no transaction setup required.
    @param  i  true = inverted display, false = normal display.
*/
void TFT_ST7735::invertDisplay(bool i) {
  startWrite();
  writeCommand(i ? invertOnCommand : invertOffCommand);
  endWrite();
}

/*!
    @brief   Given 8-bit red, green and blue values, return a 'packed'
             16-bit color value in '565' RGB format (5 bits red, 6 bits
             green, 5 bits blue). This is just a mathematical operation,
             no hardware is touched.
    @param   red    8-bit red brightnesss (0 = off, 255 = max).
    @param   green  8-bit green brightnesss (0 = off, 255 = max).
    @param   blue   8-bit blue brightnesss (0 = off, 255 = max).
    @return  'Packed' 16-bit color value (565 format).
*/
uint16_t TFT_ST7735::color565(uint8_t red, uint8_t green, uint8_t blue) {
  return ((red & 0xF8) << 8) | ((green & 0xFC) << 3) | (blue >> 3);
}

/*!
@brief   TFT_SPITFT Send Command handles complete sending of commands and
data
@param   commandByte       The Command Byte
@param   dataBytes         A pointer to the Data bytes to send
@param   numDataBytes      The number of bytes we should send
*/
void TFT_ST7735::sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_BEGIN_TRANSACTION();
  if (_cs >= 0)
    SPI_CS_LOW();

  SPI_DC_LOW();          // Command mode
  spiWrite(commandByte); // Send the command byte

  SPI_DC_HIGH();
  for (int i = 0; i < numDataBytes; i++) {
    spiWrite(*dataBytes); // Send the data bytes
    dataBytes++;
  }

  if (_cs >= 0)
    SPI_CS_HIGH();
  SPI_END_TRANSACTION();
}

/*!
 @brief   TFT_SPITFT Send Command handles complete sending of commands and
 data
 @param   commandByte       The Command Byte
 @param   dataBytes         A pointer to the Data bytes to send
 @param   numDataBytes      The number of bytes we should send
 */
void TFT_ST7735::sendCommand(uint8_t commandByte, const uint8_t *dataBytes,
                                  uint8_t numDataBytes) {
  SPI_BEGIN_TRANSACTION();
  if (_cs >= 0)
    SPI_CS_LOW();

  SPI_DC_LOW();          // Command mode
  spiWrite(commandByte); // Send the command byte

  SPI_DC_HIGH();
  for (int i = 0; i < numDataBytes; i++) {
    spiWrite(pgm_read_byte(dataBytes++));
  }

  if (_cs >= 0)
    SPI_CS_HIGH();
  SPI_END_TRANSACTION();
}

/*!
 @brief  TFT_SPITFT sendCommand16 handles complete sending of
         commands and data for 16-bit parallel displays. Currently somewhat
         rigged for the NT35510, which has the odd behavior of wanting
         commands 16-bit, but subsequent data as 8-bit values, despite
         the 16-bit bus (high byte is always 0). Also seems to require
         issuing and incrementing address with each transfer.
 @param  commandWord   The command word (16 bits)
 @param  dataBytes     A pointer to the data bytes to send
 @param  numDataBytes  The number of bytes we should send
 */
void TFT_ST7735::sendCommand16(uint16_t commandWord,
                                    const uint8_t *dataBytes,
                                    uint8_t numDataBytes) {
  SPI_BEGIN_TRANSACTION();
  if (_cs >= 0)
    SPI_CS_LOW();

  if (numDataBytes == 0) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
  }
  for (int i = 0; i < numDataBytes; i++) {
    SPI_DC_LOW();             // Command mode
    SPI_WRITE16(commandWord); // Send the command word
    SPI_DC_HIGH();            // Data mode
    commandWord++;
    SPI_WRITE16((uint16_t)pgm_read_byte(dataBytes++));
  }

  if (_cs >= 0)
    SPI_CS_HIGH();
  SPI_END_TRANSACTION();
}

/*!
 @brief   Read 8 bits of data from display configuration memory (not RAM).
 This is highly undocumented/supported and should be avoided,
 function is only included because some of the examples use it.
 @param   commandByte
 The command register to read data from.
 @param   index
 The byte index into the command to read from.
 @return  Unsigned 8-bit data read from display register.
 */
/**************************************************************************/
uint8_t TFT_ST7735::readcommand8(uint8_t commandByte, uint8_t index) {
  uint8_t result;
  startWrite();
  SPI_DC_LOW(); // Command mode
  spiWrite(commandByte);
  SPI_DC_HIGH(); // Data mode
  do {
    result = spiRead();
  } while (index--); // Discard bytes up to index'th
  endWrite();
  return result;
}

/*!
 @brief   Read 16 bits of data from display register.
          For 16-bit parallel displays only.
 @param   addr  Command/register to access.
 @return  Unsigned 16-bit data.
 */
uint16_t TFT_ST7735::readcommand16(uint16_t addr) {
  uint16_t result = 0;
  return result;
}

// -------------------------------------------------------------------------
// Lowest-level hardware-interfacing functions. Many of these are inline and
// compile to different things based on #defines -- typically just a few
// instructions. Others, not so much, those are not inlined.

/*!
    @brief  Start an SPI transaction if using the hardware SPI interface to
            the display. If using an earlier version of the Arduino platform
            (before the addition of SPI transactions), this instead attempts
            to set up the SPI clock and mode. No action is taken if the
            connection is not hardware SPI-based. This does NOT include a
            chip-select operation -- see startWrite() for a function that
            encapsulated both actions.
*/
inline void TFT_ST7735::SPI_BEGIN_TRANSACTION(void) {
  if (connection == TFT_HARD_SPI) {
    hwspi._spi->beginTransaction(hwspi.settings);
  }
}

/*!
    @brief  End an SPI transaction if using the hardware SPI interface to
            the display. No action is taken if the connection is not
            hardware SPI-based or if using an earlier version of the Arduino
            platform (before the addition of SPI transactions). This does
            NOT include a chip-deselect operation -- see endWrite() for a
            function that encapsulated both actions.
*/
inline void TFT_ST7735::SPI_END_TRANSACTION(void) {
  if (connection == TFT_HARD_SPI) {
    hwspi._spi->endTransaction();
  }
}

/*!
    @brief  Issue a single 8-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the byte. This is another of
            those functions in the library with a now-not-accurate name
            that's being maintained for compatibility with outside code.
            This function is used even if display connection is parallel.
    @param  b  8-bit value to write.
*/
void TFT_ST7735::spiWrite(uint8_t b) {
  if (connection == TFT_HARD_SPI) {
#if defined(__AVR__)
    AVR_WRITESPI(b);
#elif defined(ESP8266) || defined(ESP32)
    hwspi._spi->write(b);
#elif defined(ARDUINO_ARCH_RP2040)
    spi_inst_t *pi_spi = hwspi._spi == &SPI ? spi0 : spi1;
    spi_write_blocking(pi_spi, &b, 1);
#else
    hwspi._spi->transfer(b);
#endif
  }
}

/*!
    @brief  Write a single command byte to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. There is no corresponding explicit writeData()
            function -- just use spiWrite().
    @param  cmd  8-bit command to write.
*/
void TFT_ST7735::writeCommand(uint8_t cmd) {
  SPI_DC_LOW();
  spiWrite(cmd);
  SPI_DC_HIGH();
}

/*!
    @brief   Read a single 8-bit value from the display. Chip-select and
             transaction must have been previously set -- this ONLY reads
             the byte. This is another of those functions in the library
             with a now-not-accurate name that's being maintained for
             compatibility with outside code. This function is used even if
             display connection is parallel.
    @return  Unsigned 8-bit value read (always zero if USE_FAST_PINIO is
             not supported by the MCU architecture).
*/
uint8_t TFT_ST7735::spiRead(void) {
  uint8_t b = 0;
  uint16_t w = 0;
  if (connection == TFT_HARD_SPI) {
    return hwspi._spi->transfer((uint8_t)0);
  }
}

/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word.
            Thus operates ONLY on 'wide' (16-bit) parallel displays!
    @param  w  16-bit value to write.
*/
void TFT_ST7735::write16(uint16_t w) {
}

/*!
    @brief  Write a single command word to the display. Chip-select and
            transaction must have been previously set -- this ONLY sets
            the device to COMMAND mode, issues the byte and then restores
            DATA mode. This operates ONLY on 'wide' (16-bit) parallel
            displays!
    @param  cmd  16-bit command to write.
*/
void TFT_ST7735::writeCommand16(uint16_t cmd) {
  SPI_DC_LOW();
  write16(cmd);
  SPI_DC_HIGH();
}

/*!
    @brief   Read a single 16-bit value from the display. Chip-select and
             transaction must have been previously set -- this ONLY reads
             the byte. This operates ONLY on 'wide' (16-bit) parallel
             displays!
    @return  Unsigned 16-bit value read (always zero if USE_FAST_PINIO is
             not supported by the MCU architecture).
*/
uint16_t TFT_ST7735::read16(void) {
  uint16_t w = 0;
  return w;
}

/*!
    @brief  Set the software (bitbang) SPI MOSI line HIGH.
*/
inline void TFT_ST7735::SPI_MOSI_HIGH(void) {
  *swspi.mosiPort |= swspi.mosiPinMaskSet;
}

/*!
    @brief  Set the software (bitbang) SPI MOSI line LOW.
*/
inline void TFT_ST7735::SPI_MOSI_LOW(void) {
  *swspi.mosiPort &= swspi.mosiPinMaskClr;
}

/*!
    @brief  Set the software (bitbang) SPI SCK line HIGH.
*/
inline void TFT_ST7735::SPI_SCK_HIGH(void) {
  *swspi.sckPort |= swspi.sckPinMaskSet;
}

/*!
    @brief  Set the software (bitbang) SPI SCK line LOW.
*/
inline void TFT_ST7735::SPI_SCK_LOW(void) {
  *swspi.sckPort &= swspi.sckPinMaskClr;
}

/*!
    @brief   Read the state of the software (bitbang) SPI MISO line.
    @return  true if HIGH, false if LOW.
*/
inline bool TFT_ST7735::SPI_MISO_READ(void) {
  return *swspi.misoPort & swspi.misoPinMask;
}

/*!
    @brief  Issue a single 16-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the word. Despite the name,
            this function is used even if display connection is parallel;
            name was maintaned for backward compatibility. Naming is also
            not consistent with the 8-bit version, spiWrite(). Sorry about
            that. Again, staying compatible with outside code.
    @param  w  16-bit value to write.
*/
void TFT_ST7735::SPI_WRITE16(uint16_t w) {
  if (connection == TFT_HARD_SPI) {
#if defined(__AVR__)
    AVR_WRITESPI(w >> 8);
    AVR_WRITESPI(w);
#elif defined(ESP8266) || defined(ESP32)
    hwspi._spi->write16(w);
#elif defined(ARDUINO_ARCH_RP2040)
    spi_inst_t *pi_spi = hwspi._spi == &SPI ? spi0 : spi1;
    w = __builtin_bswap16(w);
    spi_write_blocking(pi_spi, (uint8_t *)&w, 2);
#else
    // MSB, LSB because TFTs are generally big-endian
    hwspi._spi->transfer(w >> 8);
    hwspi._spi->transfer(w);
#endif
  }
}

/*!
    @brief  Issue a single 32-bit value to the display. Chip-select,
            transaction and data/command selection must have been
            previously set -- this ONLY issues the longword. Despite the
            name, this function is used even if display connection is
            parallel; name was maintaned for backward compatibility. Naming
            is also not consistent with the 8-bit version, spiWrite().
            Sorry about that. Again, staying compatible with outside code.
    @param  l  32-bit value to write.
*/
void TFT_ST7735::SPI_WRITE32(uint32_t l) {
  if (connection == TFT_HARD_SPI) {
#if defined(__AVR__)
    AVR_WRITESPI(l >> 24);
    AVR_WRITESPI(l >> 16);
    AVR_WRITESPI(l >> 8);
    AVR_WRITESPI(l);
#elif defined(ESP8266) || defined(ESP32)
    hwspi._spi->write32(l);
#elif defined(ARDUINO_ARCH_RP2040)
    spi_inst_t *pi_spi = hwspi._spi == &SPI ? spi0 : spi1;
    l = __builtin_bswap32(l);
    spi_write_blocking(pi_spi, (uint8_t *)&l, 4);
#else
    hwspi._spi->transfer(l >> 24);
    hwspi._spi->transfer(l >> 16);
    hwspi._spi->transfer(l >> 8);
    hwspi._spi->transfer(l);
#endif
  }
}

/*!
    @brief  Set the WR line LOW, then HIGH. Used for parallel-connected
            interfaces when writing data.
*/
inline void TFT_ST7735::TFT_WR_STROBE(void) {
  *tft8.wrPort &= tft8.wrPinMaskClr;
  *tft8.wrPort |= tft8.wrPinMaskSet;
}

/*!
    @brief  Set the RD line HIGH. Used for parallel-connected interfaces
            when reading data.
*/
inline void TFT_ST7735::TFT_RD_HIGH(void) {
  *tft8.rdPort |= tft8.rdPinMaskSet;
}

/*!
    @brief  Set the RD line LOW. Used for parallel-connected interfaces
            when reading data.
*/
inline void TFT_ST7735::TFT_RD_LOW(void) {
  *tft8.rdPort &= tft8.rdPinMaskClr;
}
