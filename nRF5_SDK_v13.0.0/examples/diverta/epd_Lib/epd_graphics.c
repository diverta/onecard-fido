/*
 * epd_graphics.c
 *
 *  Created on: 2016/05/13
 *		Author: K.Shoji
 */

/*******************************************************************************
 * include.
 ******************************************************************************/
#include "epd_graphics.h"
#include "epd.h"
#include "epd_config.h"
#include "gfxfont.h"
//#include "FreeSans18pt7b.h"
#include "FreeSans9pt7b.h"
#include <stdlib.h>
#include <stdint.h>


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define BUFF_SIZE		(PIXEL_COUNT / 4)


extern uint8_t tx_image_data[BUFF_SIZE + 1];

/*******************************************************************************
 * static fields.
 ******************************************************************************/
static uint8_t *image_buff = &tx_image_data[1];
static const uint8_t width = SOURCE_LINES;
static const uint8_t height = GATE_LINES;
static GFXfont *font = NULL;
static int16_t cursor_x;
static int16_t cursor_y;
static uint8_t fg_grayscale;
static uint8_t bg_grayscale;
static uint8_t text_size;
static uint8_t wrap;

/*******************************************************************************
 * function prototype.
 ******************************************************************************/

/*******************************************************************************
 * macro
 ******************************************************************************/
#define SWAP(a,b) ((a != b) && (a += b,b = a - b,a -= b))

/*******************************************************************************
 * public function.
 ******************************************************************************/
 /**
  * @brief Get address of image buffer
  */
uint8_t *get_image_buff_addr(void)
{
	return image_buff;
}


/**
 * @brief Get size of image buffer
 */
uint16_t get_image_buff_size(void)
{
	return sizeof(tx_image_data) - sizeof(uint8_t);
}


/**
 * @brief Init graphics module
 */
void epd_graphics_init(void)
{
	//font = (GFXfont *)&FreeSans24pt7b;
	//font = (GFXfont *)&FreeSans18pt7b;
	font = (GFXfont *)&FreeSans9pt7b;
	cursor_x = 0;
	cursor_y = 0;
	fg_grayscale = 0;
	bg_grayscale = 3;
	text_size = 1;
	wrap = 1;
}


/**
 * @brief Draw pixcel
 */
void draw_pixel(int16_t x, int16_t y, uint8_t grayscale)
{
	uint16_t pix_index = 0;
	uint16_t buff_index = 0;
	uint8_t shift = 0;
	uint8_t l = 0;

	if ((x < 0) || (y < 0) || (SOURCE_LINES <= x) || (GATE_LINES <= y))
	{
		return;
	}
	
	pix_index = x + y * width;
	buff_index = pix_index / 4;
	shift = 6 - (pix_index % 4) * 2;
	l = grayscale & 0x03;
	
	image_buff[buff_index] = (image_buff[buff_index] & ~(0x03 << shift)) | (l << shift);
}


/** 
 * @brief Draw Line
 */
void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t grayscale)
{
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		SWAP(x0, y0);
		SWAP(x1, y1);
	}

	if (x0 > x1) {
		SWAP(x0, x1);
		SWAP(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			draw_pixel(y0, x0, grayscale);
		} else {
			draw_pixel(x0, y0, grayscale);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}	
}


/**
 * @brief Draw Virtial Line
 */
void draw_vline(uint16_t x, uint16_t y, uint16_t h, uint8_t grayscale)
{
	draw_line(x, y, x, y + h -1, grayscale);
}


/**
 * @brief Draw Horizontal Line
 */
void draw_hline(uint16_t x, uint16_t y, uint16_t w, uint8_t grayscale)
{
	draw_line(x, y, x + w - 1, y, grayscale);
}


/**
 * @brief Draw a Rectangle
 */
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t grayscale)
{
	draw_hline(x, y, w, grayscale);
	draw_hline(x, y + h - 1, w, grayscale);
	draw_vline(x, y, h, grayscale);
	draw_vline(x + w - 1, y, h, grayscale);	
}


/**
 * @brief Fill a Rectangle
 */
void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t grayscale)
{
	for (uint16_t i = x; i < x + w; i++) {
		draw_vline(i,  y, h, grayscale);
	}
}


/**
 * @brief Draw a Circle
 */
void draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t grayscale)
{
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	draw_pixel(x0  , y0 + r, grayscale);
	draw_pixel(x0  , y0 - r, grayscale);
	draw_pixel(x0 + r, y0, grayscale);
	draw_pixel(x0 - r, y0, grayscale);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		draw_pixel(x0 + x, y0 + y, grayscale);
		draw_pixel(x0 - x, y0 + y, grayscale);
		draw_pixel(x0 + x, y0 - y, grayscale);
		draw_pixel(x0 - x, y0 - y, grayscale);
		draw_pixel(x0 + y, y0 + x, grayscale);
		draw_pixel(x0 - y, y0 + x, grayscale);
		draw_pixel(x0 + y, y0 - x, grayscale);
		draw_pixel(x0 - y, y0 - x, grayscale);
	}
}


/**
 * @brief Fill Circle
 */
void fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t grayscale)
{
	draw_vline(x0, y0 - r, 2 * r + 1, grayscale);
	fill_circle_helper(x0, y0, r, 3, 0, grayscale);	
}


/**
 * @brief 
 */
void fill_circle_helper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta, uint8_t grayscale)
{
  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

	while (x<y) {
		if (f >= 0) {
		y--;
		ddF_y += 2;
		f     += ddF_y;
		}
		x++;
		ddF_x += 2;
		f     += ddF_x;

		if (cornername & 0x1) {
			draw_vline(x0 + x, y0 - y, 2 * y + 1 + delta, grayscale);
			draw_vline(x0 + y, y0 - x, 2 * x + 1 + delta, grayscale);
		}
		if (cornername & 0x2) {
			draw_vline(x0 - x, y0 - y, 2 * y + 1 + delta, grayscale);
			draw_vline(x0 - y, y0 - x, 2 * x + 1 + delta, grayscale);
		}
	}	
}


/**
 * @brief Fill screen
 */
void fill_screen(uint8_t grayscale)
{
	uint8_t l = grayscale & 0x03;
	uint8_t fill_data = (l << 6) | (l << 4) | (l << 2) | l;
	uint16_t buff_index;
	
	for (buff_index = 0; buff_index < BUFF_SIZE; buff_index++)
	{
		image_buff[buff_index] = fill_data;
	}
}


/**
 * @brief Set cursor
 */
void set_cursor(uint16_t x, uint16_t y)
{
	cursor_x = x;
	cursor_y = y;
}


/**
 * @brief Set text grayscale
 */
void set_text_grayscale(uint8_t grayscale)
{
	fg_grayscale = grayscale & 0x03;
}


/**
 * @brief Set text background grayscale
 */
void set_text_bg_grayscale(uint8_t grayscale)
{
	bg_grayscale = grayscale & 0x03;
}


/**
 * @brief Set text size
 */
void set_text_size(uint8_t size)
{
	text_size = size;
}


/**
 * @brief Draw character
 */
void draw_char(int16_t x, int16_t y, unsigned char c, uint8_t grayscale, uint8_t size)
{
	if (font == NULL)
	{
		return;
	}
	
	c -= font->first;
	GFXglyph *glyph = &font->glyph[c];
	uint8_t *bitmap = &font->bitmap[0];
	
	uint16_t	bo = glyph->bitmapOffset;
	uint8_t		w = glyph->width;
	uint8_t		h = glyph->height;
	uint8_t		xa = glyph->xAdvance;
	int8_t		xo = glyph->xOffset;
	int8_t		yo = glyph->yOffset;
	uint8_t		xx;
	uint8_t		yy;
	uint8_t		bits;
	uint8_t		bit = 0;
	int16_t		xo16;
	int16_t		yo16;
	
	if (1 < size)
	{
		xo16 = xo;
		yo16 = yo;
	}
	
	for (yy = 0; yy < h; yy++)
	{
		for (xx = 0; xx < w; xx++)
		{
			if (!(bit++ & 7))
			{
				bits = bitmap[bo++];
			}
			if (bits & 0x80)
			{
				if (size == 1)
				{
					draw_pixel(x + xo + xx, y + yo + yy, grayscale);
				}
				else
				{
					fill_rect(x + (xo16 + xx) * size, y + (yo16 + yy) * size, size, size, grayscale);
				}
			}
			bits <<= 1;
		}
	}
}


/**
 * @brief Write a character
 */
void write(unsigned char c)
{
	if (c == '\n')
	{
		cursor_x = 0;
		cursor_y += (int16_t)text_size * (uint8_t)font->yAdvance;
	}
	else if (c != '\r')
	{
		uint8_t first = font->first;
		if ((c >= first) && (c <= (uint8_t)font->last))
		{
			uint8_t c2 = c - font->first;
			GFXglyph *glyph = &font->glyph[c2];
			uint8_t w = glyph->width;
			uint8_t h = glyph->height;
			if ((w > 0) && (h > 0))
			{
				int16_t xo = (int8_t)glyph->xOffset;
				if (wrap &&((cursor_x + text_size * (xo + w)) >= SOURCE_LINES))
				{
					cursor_x = 0;
					cursor_y += (int16_t)text_size * (uint8_t)font->yAdvance;
				}
				draw_char(cursor_x, cursor_y, c, fg_grayscale, text_size);
			}
			cursor_x += glyph->xAdvance * (int16_t)text_size;
		}
	}
}


/**
 * @brief Write string
 */
void write_str(char *c)
{
	while (*c) {
		write(*c);
		c++;
	}
}