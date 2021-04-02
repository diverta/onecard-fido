/* 
 * File:   app_board.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 16:25
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>

#include "app_main.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_board);

//
// ボタン関連
//
#define SW0_NODE	DT_ALIAS(sw0)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))

static const struct device *button;
static struct gpio_callback button_cb_data;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    if (app_main_initialized() == false) {
        return;
    }

    // for research
    LOG_DBG("Button pressed at %u", k_cycle_get_32());
}

static const struct device *initialize_button(void)
{
    const struct device *button = device_get_binding(SW0_GPIO_LABEL);
    if (button == NULL) {
        LOG_ERR("Error: didn't find %s device", SW0_GPIO_LABEL);
        return NULL;
    }

    int ret = gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d",
           ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
        return NULL;
    }

    ret = gpio_pin_interrupt_configure(button, SW0_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d",
                ret, SW0_GPIO_LABEL, SW0_GPIO_PIN);
        return NULL;
    }

    // ボタン押下時のコールバックを設定
    gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
    gpio_add_callback(button, &button_cb_data);

    // ボタンの参照を戻す
    LOG_INF("Set up button at %s pin %d", SW0_GPIO_LABEL, SW0_GPIO_PIN);
    return button;
}

//
// LED関連
//
#define LED0_NODE	DT_ALIAS(led0)
#define LED0_GPIO_LABEL	DT_GPIO_LABEL(LED0_NODE, gpios)
#define LED0_GPIO_PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define LED0_GPIO_FLAGS	(GPIO_OUTPUT | DT_GPIO_FLAGS(LED0_NODE, gpios))

static const struct device *m_led_0;

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

    // 最初は消灯しておく
    gpio_pin_set(led, pin, 0);

    // LED0の参照を戻す
    LOG_INF("Set up LED at %s pin %d", name, pin);
    return led;
}

void app_board_initialize(void)
{
    // ボタンの初期化
    button = initialize_button();
    
    // LED0の初期化
    m_led_0 = initialize_led(LED0_GPIO_LABEL, LED0_GPIO_PIN, LED0_GPIO_FLAGS);
}
