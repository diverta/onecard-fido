/* 
 * File:   tiny_tft.c
 * Author: makmorit
 *
 * Created on 2022/06/09, 17:11
 */
#include "app_tiny_tft.h"
#include "tiny_tft_const.h"
#include "tiny_tft_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(tiny_tft);
#endif

//
// データ転送関連
//
static uint8_t work_buf[16];

static bool tiny_tft_write(uint8_t b)
{
    // １バイトを転送
    work_buf[0] = b;
    return app_tiny_tft_write(work_buf, 1);
}

//
// TFT操作に必要な変数群
//
static uint8_t _colstart;       // Some displays need this changed to offset
static uint8_t _rowstart;       // Some displays need this changed to offset
static int16_t _xstart;         // Internal framebuffer X offset
static int16_t _ystart;         // Internal framebuffer Y offset
static int16_t WIDTH;           // This is the 'raw' display width - never changes
static int16_t HEIGHT;          // This is the 'raw' display height - never changes
static int16_t _width;          // Display width as modified by current rotation
static int16_t _height;         // Display height as modified by current rotation
static int16_t cursor_x;        // x location to start print()ing text
static int16_t cursor_y;        // y location to start print()ing text
static uint16_t textcolor;      // 16-bit background color for print()
static uint16_t textbgcolor;    // 16-bit text color for print()
static uint8_t textsize_x;      // Desired magnification in X-axis of text to print()
static uint8_t textsize_y;      // Desired magnification in Y-axis of text to print()
static uint8_t orientation;     // Display rotation (0 thru 3)
static bool wrap;               // If set, 'wrap' text at right edge of display
static bool _cp437;             // If set, use correct CP437 charset (default is off)

//
// TFTディスプレイ初期化関連
//
static void tiny_tft_initialize(void)
{
    // Initialization values for graphics
    WIDTH       = ST7735_TFTWIDTH_80;
    HEIGHT      = ST7735_TFTHEIGHT_160;
    _width      = WIDTH;
    _height     = HEIGHT;
    orientation = 0;
    cursor_x    = 0;
    cursor_y    = 0;
    textsize_x  = 1;
    textsize_y  = 1;
    textcolor   = 0xFFFF;
    textbgcolor = 0xFFFF;
    wrap        = true;
    _cp437      = false;
}

static void begin_spi(uint32_t freq) 
{
    // Initialize spi config
    app_tiny_tft_initialize(freq);

    // Init basic control pins common to all connection types
    app_tiny_tft_set_c_s(HIGH);
    app_tiny_tft_set_d_c(HIGH);

    // Perform reset
    app_tiny_tft_set_rst(HIGH);
    app_tiny_tft_delay_ms(100);
    app_tiny_tft_set_rst(LOW);
    app_tiny_tft_delay_ms(100);
    app_tiny_tft_set_rst(HIGH);
    app_tiny_tft_delay_ms(200);
}

static void send_command(uint8_t command_byte, uint8_t *data_bytes, uint8_t data_size) 
{
    // Send the command byte
    app_tiny_tft_set_d_c(LOW);
    tiny_tft_write(command_byte);

    // Send the data bytes
    app_tiny_tft_set_d_c(HIGH);
    if (data_size > 0) {
        app_tiny_tft_write(data_bytes, data_size);
    }
}

static void initialize_display(uint8_t *addr) 
{
    uint16_t offset = 0;
    uint16_t ms;
    uint8_t command_num;
    uint8_t cmd;
    uint8_t arg_num;

    // Number of commands to follow
    command_num = addr[offset++];

    // For each command...
    while (command_num--) {
        // Read command
        cmd = addr[offset++];
        // Number of args to follow
        arg_num = addr[offset++];
        // If high-bit set, delay follows args
        ms = arg_num & ST_CMD_DELAY;
        // Mask out delay bit
        arg_num &= ~ST_CMD_DELAY;
        send_command(cmd, addr + offset, arg_num);
        offset += arg_num;

        if (ms) {
            // Read post-command delay time (ms)
            // If 255, delay for 500 ms
            ms = addr[offset++];
            if (ms == 255) {
                ms = 500;
            }
            app_tiny_tft_delay_ms(ms);
        }
    }
}

static void set_origin_and_orientation(uint8_t orientation_) 
{
    uint8_t madctl = 0;

    // Set origin of (0,0)
    _colstart = 24;
    _rowstart = 0;

    // Set orientation of TFT display
    // can't be higher than 3
    orientation = orientation_ & 3;

    switch (orientation) {
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

        default:
            madctl = ST77XX_MADCTL_MX | ST77XX_MADCTL_MY | ST77XX_MADCTL_RGB;

            _height = ST7735_TFTHEIGHT_160;
            _width = ST7735_TFTWIDTH_80;
            _xstart = _colstart;
            _ystart = _rowstart;
            break;
    }

    send_command(ST77XX_MADCTL, &madctl, 1);
}

//
// TFTディスプレイを初期化
//
void tiny_tft_init_display(void)
{
    // Initialization values for graphics
    tiny_tft_initialize();
    
    // Default SPI data clock frequency
    begin_spi(1000000);

    // Initialization code
    initialize_display(tiny_tft_const_init_command_1());
    initialize_display(tiny_tft_const_init_command_2());
    initialize_display(tiny_tft_const_init_command_3());

    // Change MADCTL color filter
    uint8_t data = 0xC0;
    send_command(ST77XX_MADCTL, &data, 1);

    // Set origin of (0,0) and orientation of TFT display
    set_origin_and_orientation(0);
}
