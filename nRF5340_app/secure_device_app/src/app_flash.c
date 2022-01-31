/* 
 * File:   app_flash.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 16:51
 */
#include <stdio.h>
#include <zephyr/types.h>
#include <zephyr.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(app_flash);

//
// for `nvs_calc_free_space`
//
#include <settings/settings_nvs.h>

//
// `nvs_calc_free_space`の引数
// （ファイルシステムのアドレスポインター）を格納する
// `zephyr/subsys/settings/src/settings_store.c`上の変数
//
extern struct settings_store *settings_save_dst;

static bool get_settings_nvs_stat(size_t *total_size, size_t *free_size)
{
    struct settings_nvs *cf = (struct settings_nvs *)settings_save_dst;
    struct nvs_fs *cf_nvs = &cf->cf_nvs;

    // Total size
    *total_size = cf_nvs->sector_size * cf_nvs->sector_count;

    // Free space
    ssize_t s = nvs_calc_free_space(cf_nvs);
    if (s < 0) {
        // On error, returns negative value of 
        // errno.h defined error codes.
        LOG_ERR("nvs_calc_free_space returns %d", s);
        *free_size = 0;
        return false;

    } else {
        // On success, it will be equal to the number of 
        // bytes that can still be written to the file system. 
        *free_size = (size_t)s;
        return true;
    }
}

bool app_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    // 格納領域を初期化
    memset(stat_csv_data, 0, *stat_csv_size);

    // Zephyr経由でFlash ROM統計情報を取得
    size_t total_size;
    size_t free_size;
    int corruption = 0;
    if (get_settings_nvs_stat(&total_size, &free_size) == false) {
        corruption = 1;
    }

    // 各項目をCSV化し、引数のバッファに格納
    sprintf((char *)stat_csv_data, 
        "words_available=%d,words_used=%d,corruption=%d", 
        total_size, total_size - free_size, corruption);
    *stat_csv_size = strlen((char *)stat_csv_data);
    LOG_DBG("Flash ROM statistics csv created (%d bytes)", *stat_csv_size);
    return true;
}
