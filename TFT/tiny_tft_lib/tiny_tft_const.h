/* 
 * File:   tiny_tft_const.h
 * Author: makmorit
 *
 * Created on 2022/06/12, 10:02
 */
#ifndef TINY_TFT_CONST_H
#define TINY_TFT_CONST_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
uint8_t    *tiny_tft_const_init_command_1(void);
uint8_t    *tiny_tft_const_init_command_2(void);
uint8_t    *tiny_tft_const_init_command_3(void);
uint8_t    *tiny_tft_const_raster_font(void);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_CONST_H */
