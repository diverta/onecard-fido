/* 
 * File:   rtcc.c
 * Author: makmorit
 *
 * Created on 2022/11/16, 10:16
 */
// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// プラットフォーム依存コード
#include "app_rtcc.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(rtcc);
#endif

// モジュール利用の可否を保持
#ifdef CONFIG_USE_EXTERNAL_RTCC
static bool rtcc_is_available = true;
#else
static bool rtcc_is_available = false;
#endif

// 作業領域
static char work_buf[32];

void rtcc_init(void)
{
    // RTCCが搭載されていない場合は終了
    if (rtcc_is_available == false) {
        LOG_INF("RTCC is unavailable");
        return;
    }
    
    // RTCCの初期化
    if (app_rtcc_initialize() == false) {
        return;
    }
    if (app_rtcc_get_timestamp(work_buf, sizeof(work_buf)) == false) {
        return;
    }

    // 現在時刻を表示
    LOG_INF("RTCC is available. Current timestamp: %s", work_buf);
}

bool rtcc_update_timestamp_by_unixtime(uint32_t unixtime, uint8_t timezone_diff_hours)
{
    // RTCCが搭載されていない場合は終了
    if (rtcc_is_available == false) {
        LOG_ERR("RTCC is unavailable");
        return false;
    }

    // カウンターをRTCCに設定
    if (app_rtcc_set_timestamp(unixtime, timezone_diff_hours) == false) {
        return false;
    }

    // 設定された現在時刻を取得
    if (app_rtcc_get_timestamp(work_buf, sizeof(work_buf)) == false) {
        return false;
    }

    // 現在時刻を表示
    LOG_INF("Current timestamp (updated by unixtime): %s", work_buf);
    return true;
}

bool rtcc_get_timestamp_string(char *buf, size_t size)
{
    // 設定された現在時刻を取得
    return app_rtcc_get_timestamp(buf, size);
}
