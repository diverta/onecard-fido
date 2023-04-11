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
void        tiny_tft_base_start_reset(void);
void        tiny_tft_base_end_reset(void);
void        tiny_tft_base_start_write(void);
void        tiny_tft_base_end_write(void);
void        tiny_tft_base_delay_ms(uint32_t ms);
void        tiny_tft_base_init(void);
bool        tiny_tft_base_write_byte(uint8_t b);
bool        tiny_tft_base_write_dword(uint32_t l);
void        tiny_tft_base_write_command(uint8_t command_byte);
void        tiny_tft_base_write_data(uint8_t command_byte, uint8_t *data_bytes, uint8_t data_size);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_BASE_H */
