/* 
 * File:   app_flash.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 16:51
 */
#include <zephyr/types.h>
#include <zephyr.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_flash);

bool app_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    // TODO: 後日再実装
    (void)stat_csv_data;
    (void)stat_csv_size;
    return false;
}
