#ifndef _TFT_ST7735H_
#define _TFT_ST7735H_

#include "Arduino.h"
#include "Print.h"
#include <SPI.h>

#define ST7735_TFTWIDTH_80 80
#define ST7735_TFTHEIGHT_160 160

// special signifier for command lists
#define ST_CMD_DELAY 0x80

#define ST77XX_NOP 0x00
#define ST77XX_SWRESET 0x01
#define ST77XX_RDDID 0x04
#define ST77XX_RDDST 0x09

#define ST77XX_SLPIN 0x10
#define ST77XX_SLPOUT 0x11
#define ST77XX_PTLON 0x12
#define ST77XX_NORON 0x13

#define ST77XX_INVOFF 0x20
#define ST77XX_INVON 0x21
#define ST77XX_DISPOFF 0x28
#define ST77XX_DISPON 0x29
#define ST77XX_CASET 0x2A
#define ST77XX_RASET 0x2B
#define ST77XX_RAMWR 0x2C
#define ST77XX_RAMRD 0x2E

#define ST77XX_PTLAR 0x30
#define ST77XX_TEOFF 0x34
#define ST77XX_TEON 0x35
#define ST77XX_MADCTL 0x36
#define ST77XX_COLMOD 0x3A

#define ST77XX_MADCTL_MY 0x80
#define ST77XX_MADCTL_MX 0x40
#define ST77XX_MADCTL_MV 0x20
#define ST77XX_MADCTL_ML 0x10
#define ST77XX_MADCTL_RGB 0x00

#define ST77XX_RDID1 0xDA
#define ST77XX_RDID2 0xDB
#define ST77XX_RDID3 0xDC
#define ST77XX_RDID4 0xDD

// Some ready-made 16-bit ('565') color settings:
#define ST77XX_BLACK 0x0000
#define ST77XX_WHITE 0xFFFF
#define ST77XX_RED 0xF800
#define ST77XX_GREEN 0x07E0
#define ST77XX_BLUE 0x001F
#define ST77XX_CYAN 0x07FF
#define ST77XX_MAGENTA 0xF81F
#define ST77XX_YELLOW 0xFFE0
#define ST77XX_ORANGE 0xFC00

// Some register settings
#define ST7735_MADCTL_BGR 0x08
#define ST7735_MADCTL_MH 0x04

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR 0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1 0xC0
#define ST7735_PWCTR2 0xC1
#define ST7735_PWCTR3 0xC2
#define ST7735_PWCTR4 0xC3
#define ST7735_PWCTR5 0xC4
#define ST7735_VMCTR1 0xC5

#define ST7735_PWCTR6 0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Some ready-made 16-bit ('565') color settings:
#define ST7735_BLACK ST77XX_BLACK
#define ST7735_WHITE ST77XX_WHITE
#define ST7735_RED ST77XX_RED
#define ST7735_GREEN ST77XX_GREEN
#define ST7735_BLUE ST77XX_BLUE
#define ST7735_CYAN ST77XX_CYAN
#define ST7735_MAGENTA ST77XX_MAGENTA
#define ST7735_YELLOW ST77XX_YELLOW
#define ST7735_ORANGE ST77XX_ORANGE

/// Font data stored PER GLYPH
typedef struct {
  uint16_t bitmapOffset;  ///< Pointer into GRPXfont->bitmap
  uint8_t width;          ///< Bitmap dimensions in pixels
  uint8_t height;         ///< Bitmap dimensions in pixels
  uint8_t xAdvance;       ///< Distance to advance cursor (x axis)
  int8_t xOffset;         ///< X dist from cursor pos to UL corner
  int8_t yOffset;         ///< Y dist from cursor pos to UL corner
} GRPXglyph;

/// Data stored for FONT AS A WHOLE
typedef struct {
  uint8_t *bitmap;        ///< Glyph bitmaps, concatenated
  GRPXglyph *glyph;       ///< Glyph array
  uint16_t first;         ///< ASCII extents (first char)
  uint16_t last;          ///< ASCII extents (last char)
  uint8_t yAdvance;       ///< Newline distance (y axis)
} GRPXfont;

// HARDWARE CONFIG
// PORT values are 8-bit
typedef uint8_t ADAGFX_PORT_t;

// PORT register type
typedef volatile ADAGFX_PORT_t *PORTreg_t;

// Use direct PORT register access
#define USE_FAST_PINIO

// Hardware SPI default speed
#define DEFAULT_SPI_FREQ 8000000L

class TFT_ST7735 : public Print {
public:
  TFT_ST7735(uint16_t w, uint16_t h, int8_t CS, int8_t RS, int8_t RST = -1);

  void setAddrWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  void enableDisplay(boolean enable);
  void enableTearing(boolean enable);
  void enableSleep(boolean enable);
  
  // Differences between displays (usu. identified by colored tab on
  // plastic overlay) are odd enough that we need to do this 'by hand':
  void initR(void); 
  void setRotation(uint8_t m);

  // Subclass' begin() function invokes this to initialize hardware.
  // freq=0 to use default SPI speed. spiMode must be one of the SPI_MODEn
  // values defined in SPI.h, which are NOT the same as 0 for SPI_MODE0,
  // 1 for SPI_MODE1, etc...use ONLY the SPI_MODEn defines! Only!
  // Name is outdated (interface may be parallel) but for compatibility:
  void initSPI(uint32_t freq = 0, uint8_t spiMode = SPI_MODE0);
  void setSPISpeed(uint32_t freq);
  // Chip select and/or hardware SPI transaction start as needed:
  void startWrite(void);
  // Chip deselect and/or hardware SPI transaction end as needed:
  void endWrite(void);
  void sendCommand(uint8_t commandByte, uint8_t *dataBytes,
                   uint8_t numDataBytes);
  void sendCommand(uint8_t commandByte, const uint8_t *dataBytes = NULL,
                   uint8_t numDataBytes = 0);
  void sendCommand16(uint16_t commandWord, const uint8_t *dataBytes = NULL,
                     uint8_t numDataBytes = 0);
  uint8_t readcommand8(uint8_t commandByte, uint8_t index = 0);
  uint16_t readcommand16(uint16_t addr);

  // These functions require a chip-select and/or SPI transaction
  // around them. Higher-level graphics primitives might start a
  // single transaction and then make multiple calls to these functions
  // (e.g. circle or text rendering might make repeated lines or rects)
  // before ending the transaction. It's more efficient than starting a
  // transaction every time.
  void writePixel(int16_t x, int16_t y, uint16_t color);
  void writePixels(uint16_t *colors, uint32_t len, bool block = true,
                   bool bigEndian = false);
  void writeColor(uint16_t color, uint32_t len);
  void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,
                     uint16_t color);
  void writeFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void writeFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  // This is a new function, similar to writeFillRect() except that
  // all arguments MUST be onscreen, sorted and clipped. If higher-level
  // primitives can handle their own sorting/clipping, it avoids repeating
  // such operations in the low-level code, making it potentially faster.
  // CALLING THIS WITH UNCLIPPED OR NEGATIVE VALUES COULD BE DISASTROUS.
  inline void writeFillRectPreclipped(int16_t x, int16_t y, int16_t w,
                                      int16_t h, uint16_t color);
  // Another new function, companion to the new non-blocking
  // writePixels() variant.
  void dmaWait(void);
  // Used by writePixels() in some situations, but might have rare need in
  // user code, so it's public...
  bool dmaBusy(void) const; // true if DMA is used and busy, false otherwise
  void swapBytes(uint16_t *src, uint32_t len, uint16_t *dest = NULL);

  // These functions are similar to the 'write' functions above, but with
  // a chip-select and/or SPI transaction built-in. They're typically used
  // solo -- that is, as graphics primitives in themselves, not invoked by
  // higher-level primitives (which should use the functions above).
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
  // A single-pixel push encapsulated in a transaction. I don't think
  // this is used anymore (BMP demos might've used it?) but is provided
  // for backward compatibility, consider it deprecated:
  void pushColor(uint16_t color);

  void invertDisplay(bool i);
  uint16_t color565(uint8_t r, uint8_t g, uint8_t b);

  // Despite parallel additions, function names kept for compatibility:
  void spiWrite(uint8_t b);          // Write single byte as DATA
  void writeCommand(uint8_t cmd);    // Write single byte as COMMAND
  uint8_t spiRead(void);             // Read single byte of data
  void write16(uint16_t w);          // Write 16-bit value as DATA
  void writeCommand16(uint16_t cmd); // Write 16-bit value as COMMAND
  uint16_t read16(void);             // Read single 16-bit value

  // Most of these low-level functions were formerly macros in
  // TFT_SPITFT_Macros.h. Some have been made into inline functions
  // to avoid macro mishaps. Despite the addition of code for a parallel
  // display interface, the names have been kept for backward
  // compatibility (some subclasses may be invoking these):
  void SPI_WRITE16(uint16_t w); // Not inline
  void SPI_WRITE32(uint32_t l); // Not inline
  // Old code had both a spiWrite16() function and SPI_WRITE16 macro
  // in addition to the SPI_WRITE32 macro. The latter two have been
  // made into functions here, and spiWrite16() removed (use SPI_WRITE16()
  // instead). It looks like most subclasses had gotten comfortable with
  // SPI_WRITE16 and SPI_WRITE32 anyway so those names were kept rather
  // than the less-obnoxious camelcase variants, oh well.

  // Placing these functions entirely in the class definition inlines
  // them implicitly them while allowing their use in other code:

  /*!
      @brief  Set the chip-select line HIGH. Does NOT check whether CS pin
              is set (>=0), that should be handled in calling function.
              Despite function name, this is used even if the display
              connection is parallel.
  */
  void SPI_CS_HIGH(void) {
    *csPort |= csPinMaskSet;
  }

  /*!
      @brief  Set the chip-select line LOW. Does NOT check whether CS pin
              is set (>=0), that should be handled in calling function.
              Despite function name, this is used even if the display
              connection is parallel.
  */
  void SPI_CS_LOW(void) {
    *csPort &= csPinMaskClr;
  }

  /*!
      @brief  Set the data/command line HIGH (data mode).
  */
  void SPI_DC_HIGH(void) {
    *dcPort |= dcPinMaskSet;
  }

  /*!
      @brief  Set the data/command line LOW (command mode).
  */
  void SPI_DC_LOW(void) {
    *dcPort &= dcPinMaskClr;
  }

  // TRANSACTION API / CORE DRAW API
  void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);

  // BASIC DRAW API
  void fillScreen(uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void drawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint16_t color);
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
  void fillCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint16_t color);
  void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
  void drawRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  void fillRoundRect(int16_t x0, int16_t y0, int16_t w, int16_t h, int16_t radius, uint16_t color);
  void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
  void drawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color, uint16_t bg);
  void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
  void drawBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bg);
  void drawXBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h, uint16_t color);
  void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h);
  void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, int16_t w, int16_t h);
  void drawGrayscaleBitmap(int16_t x, int16_t y, const uint8_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
  void drawGrayscaleBitmap(int16_t x, int16_t y, uint8_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
  void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t w, int16_t h);
  void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, int16_t w, int16_t h);
  void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], const uint8_t mask[], int16_t w, int16_t h);
  void drawRGBBitmap(int16_t x, int16_t y, uint16_t *bitmap, uint8_t *mask, int16_t w, int16_t h);
  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y);
  void getTextBounds(const char *string, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
  void getTextBounds(const __FlashStringHelper *s, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
  void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h);
  void setTextSize(uint8_t s);
  void setTextSize(uint8_t sx, uint8_t sy);
  void setFont(const GRPXfont *f = NULL);

  /*!
    @brief  Set text cursor location
    @param  x    X coordinate in pixels
    @param  y    Y coordinate in pixels
  */
  void setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
  }

  /*!
    @brief   Set text font color with transparant background
    @param   c   16-bit 5-6-5 Color to draw text with
    @note    For 'transparent' background, background and foreground
             are set to same color rather than using a separate flag.
  */
  void setTextColor(uint16_t c) { textcolor = textbgcolor = c; }

  /*!
    @brief   Set text font color with custom background color
    @param   c   16-bit 5-6-5 Color to draw text with
    @param   bg  16-bit 5-6-5 Color to draw background/fill with
  */
  void setTextColor(uint16_t c, uint16_t bg) {
    textcolor = c;
    textbgcolor = bg;
  }

  /*!
  @brief  Set whether text that is too long for the screen width should
          automatically wrap around to the next line (else clip right).
  @param  w  true for wrapping, false for clipping
  */
  void setTextWrap(bool w) { wrap = w; }

  /*!
    @brief  Enable (or disable) Code Page 437-compatible charset.
            There was an error in glcdfont.c for the longest time -- one
            character (#176, the 'light shade' block) was missing -- this
            threw off the index of every character that followed it.
            But a TON of code has been written with the erroneous
            character indices. By default, the library uses the original
            'wrong' behavior and old sketches will still work. Pass
            'true' to this function to use correct CP437 character values
            in your code.
    @param  x  true = enable (new behavior), false = disable (old behavior)
  */
  void cp437(bool x = true) { _cp437 = x; }

  using Print::write;
  virtual size_t write(uint8_t);

  /*!
    @brief      Get width of the display, accounting for current rotation
    @returns    Width in pixels
  */
  int16_t width(void) const { return _width; };

  /*!
    @brief      Get height of the display, accounting for current rotation
    @returns    Height in pixels
  */
  int16_t height(void) const { return _height; }

  /*!
    @brief      Get rotation setting for display
    @returns    0 thru 3 corresponding to 4 cardinal rotations
  */
  uint8_t getRotation(void) const { return rotation; }

  // get current cursor position (get rotation safe maximum values,
  // using: width() for x, height() for y)
  /*!
    @brief  Get text cursor X location
    @returns    X coordinate in pixels
  */
  int16_t getCursorX(void) const { return cursor_x; }

  /*!
    @brief      Get text cursor Y location
    @returns    Y coordinate in pixels
  */
  int16_t getCursorY(void) const { return cursor_y; };

protected:
  uint8_t _colstart = 0,   ///< Some displays need this changed to offset
      _rowstart = 0,       ///< Some displays need this changed to offset
      spiMode = SPI_MODE0; ///< Certain display needs MODE3 instead

  void begin(uint32_t freq = 0);
  void commonInit(const uint8_t *cmdList);
  void displayInit(const uint8_t *addr);
  void setColRowStart(int8_t col, int8_t row);

  // A few more low-level member functions -- some may have previously
  // been macros. Shouldn't have a need to access these externally, so
  // they've been moved to the protected section. Additionally, they're
  // declared inline here and the code is in the .cpp file, since outside
  // code doesn't need to see these.
  inline void SPI_MOSI_HIGH(void);
  inline void SPI_MOSI_LOW(void);
  inline void SPI_SCK_HIGH(void);
  inline void SPI_SCK_LOW(void);
  inline bool SPI_MISO_READ(void);
  inline void SPI_BEGIN_TRANSACTION(void);
  inline void SPI_END_TRANSACTION(void);
  inline void TFT_WR_STROBE(void); // Parallel interface write strobe
  inline void TFT_RD_HIGH(void);   // Parallel interface read high
  inline void TFT_RD_LOW(void);    // Parallel interface read low

  // CLASS INSTANCE VARIABLES --------------------------------------------

  // Here be dragons! There's a big union of three structures here --
  // one each for hardware SPI, software (bitbang) SPI, and parallel
  // interfaces. This is to save some memory, since a display's connection
  // will be only one of these. The order of some things is a little weird
  // in an attempt to get values to align and pack better in RAM.

  PORTreg_t csPort;                 ///< PORT register for chip select
  PORTreg_t dcPort;                 ///< PORT register for data/command
  
  union {
    struct {          //   Values specific to HARDWARE SPI:
      SPIClass *_spi; ///< SPI class pointer
      SPISettings settings; ///< SPI transaction settings
      uint32_t _mode; ///< SPI data mode (transactions or no)
    } hwspi;          ///< Hardware SPI values

    struct {          //   Values specific to SOFTWARE SPI:
      PORTreg_t misoPort; ///< PORT (PIN) register for MISO
      PORTreg_t mosiPort;           ///< PORT register for MOSI
      PORTreg_t sckPort;            ///< PORT register for SCK
      ADAGFX_PORT_t mosiPinMaskSet; ///< Bitmask for MOSI SET (OR)
      ADAGFX_PORT_t mosiPinMaskClr; ///< Bitmask for MOSI CLEAR (AND)
      ADAGFX_PORT_t sckPinMaskSet;  ///< Bitmask for SCK SET (OR bitmask)
      ADAGFX_PORT_t sckPinMaskClr;  ///< Bitmask for SCK CLEAR (AND)
      ADAGFX_PORT_t misoPinMask; ///< Bitmask for MISO
      int8_t _mosi;              ///< MOSI pin #
      int8_t _miso;              ///< MISO pin #
      int8_t _sck;               ///< SCK pin #
    } swspi;                     ///< Software SPI values

    struct {                     //   Values specific to 8-bit parallel:

      volatile uint8_t *writePort;  ///< PORT register for DATA WRITE
      volatile uint8_t *readPort;   ///< PORT (PIN) register for DATA READ

      // Port direction register pointer is always 8-bit regardless of
      // PORTreg_t -- even if 32-bit port, we modify a byte-aligned 8 bits.
      volatile uint8_t *portDir;  ///< PORT direction register
      PORTreg_t wrPort;           ///< PORT register for write strobe
      PORTreg_t rdPort;           ///< PORT register for read strobe
      ADAGFX_PORT_t wrPinMaskSet; ///< Bitmask for write strobe SET (OR)
      ADAGFX_PORT_t wrPinMaskClr; ///< Bitmask for write strobe CLEAR (AND)
      ADAGFX_PORT_t rdPinMaskSet; ///< Bitmask for read strobe SET (OR)
      ADAGFX_PORT_t rdPinMaskClr; ///< Bitmask for read strobe CLEAR (AND)

      int8_t _d0;              ///< Data pin 0 #
      int8_t _wr;              ///< Write strobe pin #
      int8_t _rd;              ///< Read strobe pin # (or -1)
      bool wide = 0;           ///< If true, is 16-bit interface
    } tft8;                    ///< Parallel interface settings
  }; ///< Only one interface is active

  ADAGFX_PORT_t csPinMaskSet;     ///< Bitmask for chip select SET (OR)
  ADAGFX_PORT_t csPinMaskClr;     ///< Bitmask for chip select CLEAR (AND)
  ADAGFX_PORT_t dcPinMaskSet;     ///< Bitmask for data/command SET (OR)
  ADAGFX_PORT_t dcPinMaskClr;     ///< Bitmask for data/command CLEAR (AND)

  uint8_t connection;      ///< TFT_HARD_SPI, TFT_SOFT_SPI, etc.
  int8_t _rst;             ///< Reset pin # (or -1)
  int8_t _cs;              ///< Chip select pin # (or -1)
  int8_t _dc;              ///< Data/command pin #

  int16_t _xstart = 0;          ///< Internal framebuffer X offset
  int16_t _ystart = 0;          ///< Internal framebuffer Y offset
  uint8_t invertOnCommand = 0;  ///< Command to enable invert mode
  uint8_t invertOffCommand = 0; ///< Command to disable invert mode

  uint32_t _freq = 0;           ///< Dummy var to keep subclasses happy

  void init_graphics(int16_t w, int16_t h);
  void charBounds(unsigned char c, int16_t *x, int16_t *y, int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy);

  int16_t WIDTH;        ///< This is the 'raw' display width - never changes
  int16_t HEIGHT;       ///< This is the 'raw' display height - never changes
  int16_t _width;       ///< Display width as modified by current rotation
  int16_t _height;      ///< Display height as modified by current rotation
  int16_t cursor_x;     ///< x location to start print()ing text
  int16_t cursor_y;     ///< y location to start print()ing text
  uint16_t textcolor;   ///< 16-bit background color for print()
  uint16_t textbgcolor; ///< 16-bit text color for print()
  uint8_t textsize_x;   ///< Desired magnification in X-axis of text to print()
  uint8_t textsize_y;   ///< Desired magnification in Y-axis of text to print()
  uint8_t rotation;     ///< Display rotation (0 thru 3)
  bool wrap;            ///< If set, 'wrap' text at right edge of display
  bool _cp437;          ///< If set, use correct CP437 charset (default is off)
  GRPXfont *gfxFont;    ///< Pointer to special font
};

#endif // _TFT_ST7735H_
