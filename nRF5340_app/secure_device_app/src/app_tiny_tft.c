/* 
 * File:   app_tiny_tft.c
 * Author: makmorit
 *
 * Created on 2022/06/08, 13:58
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/spi.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_tiny_tft);

static const struct device *spi_dev;

static const struct spi_config spi_cfg = {
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8),
    .frequency = 4000000,
    .slave = 0,
};

//
// TFTの初期化
//
#include "app_tiny_tft_define.h"

static const struct device *m_tft_rst, *m_tft_d_c, *m_tft_led;

static const struct device *initialize_gpio(const char *name, gpio_pin_t pin, gpio_flags_t flags)
{
    const struct device *dev = device_get_binding(name);
    if (dev == NULL) {
        LOG_ERR("Didn't find GPIO device %s", name);
        return NULL;
    }

    int ret = gpio_pin_configure(dev, pin, flags);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure GPIO device %s pin %d", ret, name, pin);
        return NULL;
    }

    // 最初はOffに設定
    gpio_pin_set(dev, pin, 0);

    // デバイスの参照を戻す
    return dev;
}

bool app_tiny_tft_initialize(void)
{
    return true;
}

//
// デバイスの初期化
//
#ifdef CONFIG_USE_TINY_TFT
static int app_tiny_tft_init(const struct device *dev)
{
    // SPI（spi4）デバイス初期化
    (void)dev;
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi4));
    if (device_is_ready(spi_dev) == false) {
        LOG_ERR("SPI master #4 is not ready");
        return -ENOTSUP;
    }

    LOG_INF("SPI master #4 is ready");

    // 制御用GPIOデバイス初期化
    m_tft_rst = initialize_gpio(TFT_RST_GPIO_LABEL, TFT_RST_GPIO_PIN, TFT_RST_GPIO_FLAGS);
    m_tft_d_c = initialize_gpio(TFT_D_C_GPIO_LABEL, TFT_D_C_GPIO_PIN, TFT_D_C_GPIO_FLAGS);
    m_tft_led = initialize_gpio(TFT_LED_GPIO_LABEL, TFT_LED_GPIO_PIN, TFT_LED_GPIO_FLAGS);
    LOG_INF("Tiny TFT device is ready");

    return 0;
}

SYS_INIT(app_tiny_tft_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif
