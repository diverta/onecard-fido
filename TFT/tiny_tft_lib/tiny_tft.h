/* 
 * File:   tiny_tft.h
 * Author: makmorit
 *
 * Created on 2022/06/09, 17:11
 */
#ifndef TINY_TFT_H
#define TINY_TFT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        tiny_tft_init_display(void);
void        tiny_tft_fill_screen(uint16_t color);
void        tiny_tft_set_text_wrap(bool w);
void        tiny_tft_set_cursor(int16_t x, int16_t y);
void        tiny_tft_set_text_color(uint16_t c);
void        tiny_tft_set_text_size_each(uint8_t s_x, uint8_t s_y);
void        tiny_tft_set_text_size(uint8_t s);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_H */
