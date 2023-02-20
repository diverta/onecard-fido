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

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_crypto);

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

//SYS_INIT(app_crypto_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
