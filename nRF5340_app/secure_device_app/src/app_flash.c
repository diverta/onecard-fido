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
#include <stdio.h>

#include "app_flash.h"

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

bool app_flush_fs_mount(void)
{
    // マウント実行
    int rc = fs_mount(&lfs_storage_mnt);
    if (rc < 0) {
        LOG_ERR("fs_mount returns %d", rc);
        return false;
    }

    return true;
}

bool app_flush_fs_unmount(void)
{
    // マウント実行
    int rc = fs_unmount(&lfs_storage_mnt);
    if (rc < 0) {
        LOG_ERR("fs_unmount returns %d", rc);
        return false;
    }

    return true;
}

bool app_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    // ファイルシステムをマウント
    if (app_flush_fs_mount() == false) {
        return false;
    }

    // ファイルシステムの統計情報を取得
    struct fs_statvfs stat;
    int rc = fs_statvfs(lfs_storage_mnt.mnt_point, &stat);
    if (rc < 0) {
        LOG_ERR("fs_statvfs returns %d", rc);
        return false;
    }
    LOG_DBG("Flash stat (bsize=%d, bfree=%d, blocks=%d, frsize=%d)", 
            (int)stat.f_bsize, (int)stat.f_bfree, (int)stat.f_blocks, (int)stat.f_frsize);

    // 格納領域を初期化
    memset(stat_csv_data, 0, *stat_csv_size);

    // 各項目をCSV化し、引数のバッファに格納
    int words = (int)stat.f_frsize / 4;
    sprintf((char *)stat_csv_data, 
        "words_available=%d,words_used=%d,freeable_words=%d,largest_contig=%d,valid_records=%d,dirty_records=%d,corruption=%d", 
        (int)stat.f_blocks * words, 
        ((int)stat.f_blocks - (int)stat.f_bfree) * words,
        (int)stat.f_bfree * words,
        0,
        0, 
        0,
        0);
    *stat_csv_size = strlen((char *)stat_csv_data);
    LOG_DBG("Flash ROM statistics csv created (%d bytes)", *stat_csv_size);

    // ファイルシステムをアンマウント
    return app_flush_fs_unmount();
}
