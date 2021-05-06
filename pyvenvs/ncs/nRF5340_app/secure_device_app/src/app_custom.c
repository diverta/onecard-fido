/* 
 * File:   app_custom.c
 * Author: makmorit
 *
 * Created on 2021/05/06, 9:53
 */
#include <zephyr/types.h>
#include <zephyr.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_custom);

#define LOG_DEBUG_HID_REPORT        false

//
// データ処理イベント関連
//
void app_custom_hid_report_received(uint8_t *data, size_t size)
{
#if LOG_DEBUG_HID_REPORT
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "HID report");
#endif
}

void app_custom_hid_report_sent(void)
{
}
