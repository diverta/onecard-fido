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
bool        tiny_tft_is_available(void);
void        tiny_tft_base_start_reset(void);
void        tiny_tft_base_end_reset(void);
void        tiny_tft_base_start_write(void);
void        tiny_tft_base_end_write(void);
void        tiny_tft_base_delay_ms(uint32_t ms);
void        tiny_tft_base_init(void);
bool        tiny_tft_base_write_byte(uint8_t b);
bool        tiny_tft_base_write_dword(uint32_t l);
bool        tiny_tft_base_write_command(uint8_t command_byte);
bool        tiny_tft_base_write_data(uint8_t command_byte, uint8_t *data_bytes, uint8_t data_size);
void        tiny_tft_base_backlight_on(void);
void        tiny_tft_base_backlight_off(void);

#ifdef __cplusplus
}
#endif

#endif /* TINY_TFT_BASE_H */
