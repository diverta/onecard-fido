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

static bool tiny_tft_write_32(uint32_t l)
{
    // ４バイトを転送
    work_buf[0] = l >> 24;
    work_buf[1] = l >> 16;
    work_buf[2] = l >> 8;
    work_buf[3] = l;
    return app_tiny_tft_write(work_buf, 4);
}

static void tiny_tft_write_command(uint8_t command_byte) 
{
    app_tiny_tft_set_d_c(LOW);
    tiny_tft_write(command_byte);
    app_tiny_tft_set_d_c(HIGH);
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
// グラフィック操作関連
//
static void set_addr_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h) 
{
    // SPI displays set an address window rectangle 
    // for blitting pixels
    x += _xstart;
    y += _ystart;
    uint32_t xa = ((uint32_t)x << 16) | (x + w - 1);
    uint32_t ya = ((uint32_t)y << 16) | (y + h - 1);

    // Column addr set
    tiny_tft_write_command(ST77XX_CASET);
    tiny_tft_write_32(xa);

    // Row addr set
    tiny_tft_write_command(ST77XX_RASET);
    tiny_tft_write_32(ya);

    // write to RAM
    tiny_tft_write_command(ST77XX_RAMWR);
}

static void issue_color_pixels(uint16_t color, uint32_t len) 
{
    // Avoid 0-byte transfers
    if (len == 0) {
        return;
    }

    // Issue a series of pixels, all the same color
    uint8_t hi = color >> 8, lo = color;
    while (len--) {
        tiny_tft_write(hi);
        tiny_tft_write(lo);
    }
}

static void issue_color_pixel(uint16_t color) 
{
    // Issue a pixel of color
    uint8_t hi = color >> 8, lo = color;
    tiny_tft_write(hi);
    tiny_tft_write(lo);
}

static uint16_t swap_bit(uint16_t x) 
{
    uint16_t r = 0;
    uint8_t b = 16;
    while (b--) {
        r <<= 1;
        r |= (x & 1);
        x >>= 1;
    }
    return r;
}

//
// TFTディスプレイを初期化
//
void tiny_tft_init_display(void)
{
    // Initialization values for graphics
    tiny_tft_initialize();
    
    // Default SPI data clock frequency
    begin_spi(4000000);

    // Initialization code
    initialize_display(tiny_tft_const_init_command_1());
    initialize_display(tiny_tft_const_init_command_2());
    initialize_display(tiny_tft_const_init_command_3());

    // Change MADCTL color filter
    uint8_t data = 0xC0;
    send_command(ST77XX_MADCTL, &data, 1);

    // Set origin of (0,0) and orientation of TFT display
    set_origin_and_orientation(3);
}

//
// 画面全体を同一色で塗りつぶす
//
static void fill_rect_preclipped(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    // Set an address window rectangle for blitting pixels
    set_addr_window(x, y, w, h);

    // Issue a series of pixels, all the same color
    issue_color_pixels(swap_bit(color), (uint32_t)w * h);
}

static void fill_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
    // Nonzero width and height?
    if (w == 0 || h == 0) {
        return;
    }
    // If negative width...
    if (w < 0) {
        // Move X to left edge
        x += w + 1;
        // Use positive width
        w = -w;
    }
    // Not off right
    if (x >= _width) {
        return;
    }
    // If negative height...
    if (h < 0) {
        // Move Y to top edge
        y += h + 1;
        // Use positive height
        h = -h;
    }
    // Not off bottom
    if (y >= _height) {
        return;
    }
    // Not off left
    int16_t x2 = x + w - 1;
    if (x2 < 0) {
        return;
    }
    // Not off top
    int16_t y2 = y + h - 1;
    if (y2 < 0) {
        return;
    }
    // Rectangle partly or fully overlaps screen
    // Clip left
    if (x < 0) {
        x = 0;
        w = x2 + 1;
    }
    // Clip top
    if (y < 0) {
        y = 0;
        h = y2 + 1;
    }
    // Clip right
    if (x2 >= _width) {
        w = _width - x;
    }
    // Clip bottom
    if (y2 >= _height) {
        h = _height - y;
    }

    // Draw a filled rectangle to the display.
    fill_rect_preclipped(x, y, w, h, color);
}

void tiny_tft_fill_screen(uint16_t color)
{
    // Fill the screen completely with one color
    fill_rect(0, 0, _width, _height, color);
}

//
// テキスト描画関連
//
void tiny_tft_set_text_wrap(bool w)
{
    // Set whether text that is too long for the screen width should
    // automatically wrap around to the next line (else clip right).
    wrap = w;
}

void tiny_tft_set_cursor(int16_t x, int16_t y)
{
    // Set text cursor location
    // (x or y coordinate in pixels)
    cursor_x = x;
    cursor_y = y;
}

void tiny_tft_set_text_color(uint16_t c)
{
    // Set text font color with transparant background
    textcolor = textbgcolor = c;
}

void tiny_tft_set_text_size_each(uint8_t s_x, uint8_t s_y) 
{
    // Set text 'magnification' size. 
    // Each increase in s makes 1 pixel that much bigger.
    textsize_x = (s_x > 0) ? s_x : 1;
    textsize_y = (s_y > 0) ? s_y : 1;
}

void tiny_tft_set_text_size(uint8_t s) 
{
    // テキストのサイズを、引数倍の大きさに設定
    tiny_tft_set_text_size_each(s, s);
}

static void write_pixel(int16_t x, int16_t y, uint16_t color)
{
    if ((x >= 0) && (x < _width) && (y >= 0) && (y < _height)) {
        // Set an address window rectangle for blitting pixels
        set_addr_window(x, y, 1, 1);

        // Issue a pixel of color
        issue_color_pixel(swap_bit(color));
    }
}

static void write_fast_vline(int16_t x, int16_t y, int16_t h, uint16_t color)
{
    // X on screen, nonzero height
    if ((x < 0) || (x >= _width) || (h == 0)) {
        return;
    }
    // If negative height...
    if (h < 0) {                       
        // Move Y to top edge
        y += h + 1;
        // Use positive height
        h = -h;
    }
    // Not off bottom
    if (y >= _height) {
        return;
    }
    int16_t y2 = y + h - 1;
    // Not off top
    if (y2 >= 0) { 
        // Line partly or fully overlaps screen
        if (y < 0) {
            // Clip top
            y = 0;
            h = y2 + 1;
        }
        if (y2 >= _height) {
            // Clip bottom
            h = _height - y;
        }
        // Draw a filled rectangle to the display.
        fill_rect_preclipped(x, y, 1, h, color);
    }
}

static void start_write(void)
{
    app_tiny_tft_set_c_s(LOW);
}

static void end_write(void)
{
    app_tiny_tft_set_c_s(HIGH);
}

static void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size_x, uint8_t size_y)
{
    bool clip_right  = (x >= _width);
    bool clip_bottom = (y >= _height);
    bool clip_left   = ((x + 6 * size_x - 1) < 0);
    bool clip_top    = ((y + 8 * size_y - 1) < 0);
    if (clip_right || clip_bottom || clip_left || clip_top) {
        // 描画可能領域から外れている場合は処理終了
        return;
    }

    if (!_cp437 && (c >= 176)) {
        // Handle 'classic' charset behavior
        c++;
    }

    // Char bitmap = 5 columns
    start_write();
    uint8_t *font = tiny_tft_const_raster_font();
    for (int8_t i = 0; i < 5; i++) {
        uint8_t line = font[c * 5 + i];
        for (int8_t j = 0; j < 8; j++, line >>= 1) {
            if (line & 1) {
                if (size_x == 1 && size_y == 1) {
                    write_pixel(x + i, y + j, color);
                } else {
                    fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, color);
                }
            } else if (bg != color) {
                if (size_x == 1 && size_y == 1) {
                    write_pixel(x + i, y + j, bg);
                } else {
                    fill_rect(x + i * size_x, y + j * size_y, size_x, size_y, bg);
                }
            }
        }
    }

    // If opaque, draw vertical line for last column
    if (bg != color) {
        if (size_x == 1 && size_y == 1) {
            write_fast_vline(x + 5, y, 8, bg);
        } else {
            fill_rect(x + 5 * size_x, y, size_x, 8 * size_y, bg);
        }
    }
    end_write();
}

static void write(uint8_t c)
{
    // Newline?
    if (c == '\n') {
        // Reset x to zero,
        cursor_x = 0;
        // advance y one line
        cursor_y += textsize_y * 8;

    } else if (c != '\r') {
        // Ignore carriage returns
        // Off right?
        if (wrap && ((cursor_x + textsize_x * 6) > _width)) {
            // Reset x to zero,
            cursor_x = 0;
            // advance y one line
            cursor_y += textsize_y * 8;
        }
        draw_char(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize_x, textsize_y);
        // Advance x one char
        cursor_x += textsize_x * 6;
    }
}

static size_t write_buffer(const uint8_t *buffer, size_t size)
{
    size_t n = 0;
    while (size--) {
        write(*buffer++);
        n++;
    }
    return n;
}

size_t tiny_tft_print(const char *s)
{
    return write_buffer((const uint8_t *)s, strlen(s));
}
