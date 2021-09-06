/* 
 * File:   app_flash.h
 * Author: makmorit
 *
 * Created on 2021/04/06, 16:51
 */
#ifndef APP_FLASH_H
#define APP_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
bool        app_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size);

#ifdef __cplusplus
}
#endif

#endif /* APP_FLASH_H */
