/*
 * epd_graphics.h
 *
 *  Created on: 2016/05/13
 *      Author: K.Shoji
 */
#ifndef EPD_GRAPHICS_H_
#define EPD_GRAPHICS_H_

#include <stdint.h>

uint8_t *get_image_buff_addr(void);
uint16_t get_image_buff_size(void);
void epd_graphics_init(void);
void epd_graphics_update(void);
void draw_pixel(int16_t x, int16_t y, uint8_t grayscale);
void draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t grayscale);
void draw_vline(uint16_t x, uint16_t y, uint16_t h, uint8_t grayscale);
void draw_hline(uint16_t x, uint16_t y, uint16_t w, uint8_t grayscale);
void draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t grayscale);
void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t grayscale);
void draw_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t grayscale);
void fill_circle(uint16_t x0, uint16_t y0, uint16_t r, uint8_t grayscale);
void fill_circle_helper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, uint16_t delta, uint8_t grayscale);
void fill_screen(uint8_t grayscale);

void set_cursor(uint16_t x, uint16_t y);
void set_text_grayscale(uint8_t grayscale);
void set_text_bg_grayscale(uint8_t grayscale);
void set_text_size(uint8_t size);
void draw_char(int16_t x, int16_t y, unsigned char c, uint8_t grayscale, uint8_t size);
void write(unsigned char c);
void write_str(char *c);

#endif	/* EPD_GRAPHICS_H_ */
