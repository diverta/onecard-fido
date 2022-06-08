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
#include <drivers/gpio.h>

#define LED0_NODE	DT_ALIAS(tftrst)
#define LED0_GPIO_LABEL	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_GPIO_PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED0_NODE, gpios))

#define LED1_NODE	DT_ALIAS(tftdc)
#define LED1_GPIO_LABEL	DT_GPIO_LABEL(LED1_NODE, gpios)
#define LED1_GPIO_PIN	DT_GPIO_PIN(LED1_NODE, gpios)
#define LED1_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED1_NODE, gpios))

#define LED2_NODE	DT_ALIAS(tftled)
#define LED2_GPIO_LABEL	DT_GPIO_LABEL(LED2_NODE, gpios)
#define LED2_GPIO_PIN	DT_GPIO_PIN(LED2_NODE, gpios)
#define LED2_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED2_NODE, gpios))

static const struct device *m_led_0, *m_led_1, *m_led_2;

static const struct device *initialize_led(const char *name, gpio_pin_t pin, gpio_flags_t flags)
{
    const struct device *led = device_get_binding(name);
    if (led == NULL) {
        LOG_ERR("Didn't find LED device %s", name);
        return NULL;
    }

    int ret = gpio_pin_configure(led, pin, flags);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure LED device %s pin %d", ret, name, pin);
        return NULL;
    }

    // 最初はOffに設定
    gpio_pin_set(led, pin, 0);

    // デバイスの参照を戻す
    LOG_DBG("Set up LED at %s pin %d", name, pin);
    return led;
}

bool app_tiny_tft_initialize(void)
{
    gpio_pin_set(m_led_0, LED0_GPIO_PIN, 1);
    gpio_pin_set(m_led_1, LED1_GPIO_PIN, 1);
    gpio_pin_set(m_led_2, LED2_GPIO_PIN, 1);
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

    // GPIOデバイス初期化
    m_led_0 = initialize_led(LED0_GPIO_LABEL, LED0_GPIO_PIN, LED0_GPIO_FLAGS);
    m_led_1 = initialize_led(LED1_GPIO_LABEL, LED1_GPIO_PIN, LED1_GPIO_FLAGS);
    m_led_2 = initialize_led(LED2_GPIO_LABEL, LED2_GPIO_PIN, LED2_GPIO_FLAGS);

    return 0;
}

SYS_INIT(app_tiny_tft_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif
