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
#include <fs/fs.h>
#include <fs/littlefs.h>

#include "app_flash.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_flash);

// 初期動作確認用
#define WIPE_STORAGE_ON_INIT        false

//
// LittleFS file system
//
#define PARTITION_NODE DT_NODELABEL(lfs1)

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(storage);
static struct fs_mount_t lfs_storage_mnt = {
    .type = FS_LITTLEFS,
    .fs_data = &storage,
    .storage_dev = (void *)FLASH_AREA_ID(storage),
    .mnt_point = "/lfs",
};

//
// アプリケーション状態を保持
//
static bool flash_initialized = false;

void app_flash_initialize(void)
{
    // マウントポイントを取得
    struct fs_mount_t *mp = &lfs_storage_mnt;

    // マウントポイントがあるFlash領域のIDを取得
    unsigned int id = (uintptr_t)mp->storage_dev;
    const struct flash_area *pfa;

    // Flash領域を開く
    int rc = flash_area_open(id, &pfa);
    if (rc < 0) {
        LOG_ERR("Unable to find flash area %u (flash_area_open returns %d)", id, rc);
        return;
    }

#if WIPE_STORAGE_ON_INIT
    // Flash領域を消去
    rc = flash_area_erase(pfa, 0, pfa->fa_size);
    if (rc < 0) {
        LOG_ERR("Unable to erase flash area %u (flash_area_erase returns %d)", id, rc);
        return;
    }
    LOG_INF("Erasing flash area end (rc=%d)", rc);
#endif

    // Flash ROM領域を閉じる
    LOG_INF("Flash initialized (offset=0x%08x, size=%u)", (unsigned int)pfa->fa_off, (unsigned int)pfa->fa_size);
    flash_area_close(pfa);

    // 初期化完了
    flash_initialized = true;
}
