/* 
 * File:   tiny_tft.h
 * Author: makmorit
 *
 * Created on 2022/06/09, 17:11
 */
#ifndef TINY_TFT_H
#define TINY_TFT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        tiny_tft_init_display(void);
void        tiny_tft_fill_screen(uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_H */
