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

//
// デバイスの初期化
//
#ifdef CONFIG_USE_TINY_TFT
#include "app_tiny_tft_define.h"

// 制御用GPIO
static const struct device *m_tft_rst, *m_tft_d_c, *m_tft_led;

// SPI
static const struct device *spi_dev;
static struct spi_config spi_cfg;

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

//
// TFTの初期化
//
bool app_tiny_tft_initialize(uint32_t frequency)
{
    spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB;
    spi_cfg.frequency = frequency;
    spi_cfg.slave = 0;

    return true;
}

//
// データ転送関連
//
static struct spi_buf     m_tx_buf;
static struct spi_buf_set m_tx_bufs;

bool app_tiny_tft_write(uint8_t *buf, size_t len)
{
    // 転送バイトを設定
    m_tx_buf.buf = buf;
    m_tx_buf.len = len;

    m_tx_bufs.buffers = &m_tx_buf;
    m_tx_bufs.count = 1;

    int ret = spi_write(spi_dev, &spi_cfg, &m_tx_bufs);
    if (ret != 0) {
        LOG_ERR("spi_write returns %d", ret);
        return false;
    }

    return true;
}

//
// 制御用GPIO関連
//
void app_tiny_tft_set_rst(int value)
{
    gpio_pin_set(m_tft_rst, TFT_RST_GPIO_PIN, value ? 0 : 1);
}

void app_tiny_tft_set_d_c(int value)
{
    gpio_pin_set(m_tft_d_c, TFT_D_C_GPIO_PIN, value ? 0 : 1);
}

void app_tiny_tft_set_led(int value)
{
    gpio_pin_set(m_tft_led, TFT_LED_GPIO_PIN, value ? 0 : 1);
}

void app_tiny_tft_delay_ms(uint32_t ms)
{
    k_sleep(K_MSEC(ms));
}

#else

bool app_tiny_tft_initialize(void)
{
    return true;
}

bool app_tiny_tft_write(uint8_t *buf, size_t len)
{
    (void)buf;
    (void)len;
    return true;
}

void app_tiny_tft_set_rst(int value)
{
    (void)value;
}

void app_tiny_tft_set_d_c(int value)
{
    (void)value;
}

void app_tiny_tft_set_led(int value)
{
    (void)value;
}

void app_tiny_tft_delay_ms(uint32_t ms)
{
    (void)ms;
}

#endif
