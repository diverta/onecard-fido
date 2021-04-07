/* 
 * File:   app_flash.c
 * Author: makmorit
 *
 * Created on 2021/04/06, 16:51
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/flash.h>
#include <storage/flash_map.h>
#include <fs/nvs.h>

// nvs file system
static struct nvs_fs            fs;
static struct flash_pages_info  info;

#include "app_flash.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_flash);

//
// アプリケーション状態を保持
//
static bool flash_initialized = false;

void app_flash_initialize(void)
{
    // Flash ROMエリアの開始位置、ページサイズを取得
    fs.offset = FLASH_AREA_OFFSET(storage);
    const struct device *dev = device_get_binding(DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
    int rc = flash_get_page_info_by_offs(dev, fs.offset, &info);
    if (rc != 0) {
        LOG_ERR("Unable to get page info (flash_get_page_info_by_offs returns %d)", rc);
        return;
    }

    // NVSファイルシステムのセクターサイズ／数を設定
    fs.sector_size = info.size;
    fs.sector_count = 2;

    // NVSファイルシステムを初期化
    rc = nvs_init(&fs, DT_CHOSEN_ZEPHYR_FLASH_CONTROLLER_LABEL);
    if (rc != 0) {
        LOG_ERR("Flash initialize failed (nvs_init returns %d)", rc);
        return;
    }

    // 初期化完了
    LOG_DBG("Flash initialized (offset=0x%08x, sector size=%d, sector count=%d)", fs.offset, fs.sector_size, fs.sector_count);
    flash_initialized = true;
}
