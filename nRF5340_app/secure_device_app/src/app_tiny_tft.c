/* 
 * File:   app_tiny_tft.c
 * Author: makmorit
 *
 * Created on 2022/06/08, 13:58
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app_tiny_tft);

//
// デバイスの初期化
//
#ifdef CONFIG_USE_TINY_TFT
#include "app_tiny_tft_define.h"
#include "app_tiny_tft.h"

// 制御用GPIO
static const struct gpio_dt_spec m_tft_c_s = GPIO_DT_SPEC_GET_OR(DT_ALIAS(tftcs),  gpios, {0});
static const struct gpio_dt_spec m_tft_rst = GPIO_DT_SPEC_GET_OR(DT_ALIAS(tftrst), gpios, {0});
static const struct gpio_dt_spec m_tft_d_c = GPIO_DT_SPEC_GET_OR(DT_ALIAS(tftdc),  gpios, {0});
static const struct gpio_dt_spec m_tft_led = GPIO_DT_SPEC_GET_OR(DT_ALIAS(tftled), gpios, {0});

// SPI
static const struct device *spi_dev;
static struct spi_config spi_cfg;

static bool initialize_gpio(const struct gpio_dt_spec *device, gpio_flags_t flags)
{
    if (device_is_ready(device->port) == false) {
        LOG_ERR("Didn't find GPIO device %s", device->port->name);
        return false;
    }

    int ret = gpio_pin_configure_dt(device, flags);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure GPIO device %s pin %d", ret, device->port->name, device->pin);
        return false;
    }

    // 最初はOffに設定
    gpio_pin_set(device->port, device->pin, 0);
    return true;
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
    initialize_gpio(&m_tft_c_s, TFT_C_S_GPIO_FLAGS);
    initialize_gpio(&m_tft_rst, TFT_RST_GPIO_FLAGS);
    initialize_gpio(&m_tft_d_c, TFT_D_C_GPIO_FLAGS);
    initialize_gpio(&m_tft_led, TFT_LED_GPIO_FLAGS);
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

    app_tiny_tft_set_c_s(0);
    int ret = spi_write(spi_dev, &spi_cfg, &m_tx_bufs);
    app_tiny_tft_set_c_s(1);

    if (ret != 0) {
        LOG_ERR("spi_write returns %d", ret);
        return false;
    }

    return true;
}

//
// 制御用GPIO関連
//
void app_tiny_tft_set_c_s(int value)
{
    gpio_pin_set(m_tft_c_s.port, m_tft_c_s.pin, value ? 0 : 1);
}

void app_tiny_tft_set_rst(int value)
{
    gpio_pin_set(m_tft_rst.port, m_tft_rst.pin, value ? 0 : 1);
}

void app_tiny_tft_set_d_c(int value)
{
    gpio_pin_set(m_tft_d_c.port, m_tft_d_c.pin, value ? 0 : 1);
}

void app_tiny_tft_set_led(int value)
{
    gpio_pin_set(m_tft_led.port, m_tft_led.pin, value ? 0 : 1);
}

#else

bool app_tiny_tft_initialize(uint32_t frequency)
{
    (void)frequency;
    return true;
}

bool app_tiny_tft_write(uint8_t *buf, size_t len)
{
    (void)buf;
    (void)len;
    return true;
}

void app_tiny_tft_set_c_s(int value)
{
    (void)value;
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

#endif
