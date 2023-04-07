/* 
 * File:   tiny_tft_base.h
 * Author: makmorit
 *
 * Created on 2023/04/07, 17:36
 */
#ifndef TINY_TFT_BASE_H
#define TINY_TFT_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void        tiny_tft_base_start_write(void);
void        tiny_tft_base_end_write(void);
void        tiny_tft_base_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_BASE_H */
