/* 
 * File:   app_crypto.c
 * Author: makmorit
 *
 * Created on 2021/05/12, 9:59
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/entropy.h>
#include <errno.h>
#include <zephyr/init.h>

// for Mbed TLS
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/platform.h>

// for app_event_notify
#include "app_event.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_crypto);

// 関数プロトタイプ
static bool app_crypto_event_notify(uint8_t event);

//
// CTR-DRBG共有情報
//
static mbedtls_ctr_drbg_context m_drbg_ctx;

void *app_crypto_ctr_drbg_context(void)
{
    return (void *)&m_drbg_ctx;
}

//
// 初期設定
//   スタックを相当量消費するため、SYS_INITで実行します。
//
static const unsigned char ncs_seed[] = {0xde, 0xad, 0xbe, 0xef};

static int entropy_func(void *ctx, unsigned char *buf, size_t len)
{
    return entropy_get_entropy(ctx, buf, len);
}

static int app_crypto_init(const struct device *dev)
{
    // Get device binding named 'CRYPTOCELL'
    (void)dev;
    const struct device *p_device = DEVICE_DT_GET(DT_CHOSEN(zephyr_entropy));
    if (p_device == NULL) {
        LOG_ERR("device_get_binding(%s) returns NULL", p_device->name);
        return -ENODEV;
    }

    // Initialize random seed for CTR-DRBG
    mbedtls_ctr_drbg_init(&m_drbg_ctx);
    int ret = mbedtls_ctr_drbg_seed(&m_drbg_ctx, entropy_func, (void *)p_device, ncs_seed, sizeof(ncs_seed));
    if (ret != 0) {
        LOG_ERR("mbedtls_ctr_drbg_seed returns %d", ret);
        return -ENOTSUP;
    }

    LOG_INF("Mbed TLS random seed initialized");
    return 0;
}

//
// 専用スレッドの処理分岐制御
//
void app_crypto_do_process(void)
{
    // TODO: 仮の実装です。
    // 専用スレッドから`app_crypto_init`を実行
    // --> メインスレッドに制御を戻す
    app_crypto_event_notify(0x00);
}

static void app_crypto_process_for_event(uint8_t event)
{
    // TODO: 仮の実装です。
    switch (event) {
        default:
            app_crypto_init(NULL);
            break;
    }

    // メインスレッドに制御を戻す
    app_event_notify(APEVT_APP_CRYPTO_DONE);
}

//
// 暗号化関連処理の専用スレッド
//
K_FIFO_DEFINE(app_crypto_fifo);

typedef struct {
    void           *fifo_reserved;
    uint8_t         event;
} APP_CRYPTO_FIFO_T;

static bool app_crypto_event_notify(uint8_t event)
{
    // 領域を確保
    size_t size = sizeof(APP_CRYPTO_FIFO_T);
    APP_CRYPTO_FIFO_T *p_fifo = (APP_CRYPTO_FIFO_T *)k_malloc(size);
    if (p_fifo == NULL) {
        LOG_ERR("APP_CRYPTO_FIFO_T allocation failed");
        return false;
    }

    // イベントデータを待ち行列にセット
    p_fifo->event = event;
    k_fifo_put(&app_crypto_fifo, p_fifo);
    return true;
}

static void app_crypto_thread(void)
{
    while (true) {
        // イベント検知まで待機
        APP_CRYPTO_FIFO_T *p_fifo = k_fifo_get(&app_crypto_fifo, K_FOREVER);

        // イベントに対応する処理を実行
        app_crypto_process_for_event(p_fifo->event);

        // FIFOデータを解放
        k_free(p_fifo);
    }
}

K_THREAD_DEFINE(app_crypto_thread_id, 4096, app_crypto_thread, NULL, NULL, NULL, 7, 0, 0);
