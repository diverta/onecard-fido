/* 
 * File:   app_board.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 16:25
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <sys/time_units.h>
#include <power/reboot.h>

#include "app_main.h"
#include "app_board.h"
#include "app_board_define.h"
#include "app_event.h"

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_board);

#define LOG_BUTTON_INITIALIZED  false
#define LOG_BUTTON_PRESSED      false

//
// 共通処理
//
uint32_t app_board_kernel_uptime_ms_get(void)
{
    // システム起動後の通算ミリ秒数を取得
    return k_cyc_to_ms_floor32(k_cycle_get_32());
}

//
// ボタン関連
//
static const struct device *button_0,   *button_1;
static struct gpio_callback button_cb_0, button_cb_1;

static bool button_pressed(const struct device *dev, gpio_pin_t pin, int *status_pressed, uint32_t *time_pressed)
{
    if (app_main_initialized() == false) {
        return false;
    }

    // ボタン検知状態を取得
    int status_now = gpio_pin_get(dev, pin);

    // ボタン検知時刻を取得
    uint32_t time_now = app_board_kernel_uptime_ms_get();
 
    // ２回連続検知の場合は無視
    if (status_now == *status_pressed) {
#if LOG_BUTTON_PRESSED
        LOG_DBG("%s (invalid)", status_now ? "pushed" : "released");
#endif
        return false;
    }
    *status_pressed = status_now;

    // 短時間の間に検知された場合は無視
    uint32_t elapsed = time_now - *time_pressed;
    if (elapsed < 50) {
#if LOG_BUTTON_PRESSED
        LOG_DBG("%s (ignored)", status_now ? "pushed" : "released");
#endif
        return false;
    }
    *time_pressed = time_now;

#if LOG_BUTTON_PRESSED
    LOG_DBG("%s (elapsed %u msec)", status_now ? "pushed" : "released", elapsed);
#endif
    return true;
}

static void button_pressed_0(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // ボタン検知状態・検知時刻を保持
    static int status_pressed = 0;
    static uint32_t time_pressed = 0;

    // ボタン検知処理
    if (button_pressed(dev, SW0_GPIO_PIN, &status_pressed, &time_pressed) == false) {
        return;
    }

    // ボタン検知イベントを業務処理スレッドに引き渡す
    app_event_notify(status_pressed ? APEVT_BUTTON_PUSHED : APEVT_BUTTON_RELEASED);
}

static void button_pressed_1(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    // ボタン検知状態・検知時刻を保持
    static int status_pressed = 0;
    static uint32_t time_pressed = 0;

    // ボタン検知処理
    if (button_pressed(dev, SW1_GPIO_PIN, &status_pressed, &time_pressed) == false) {
        return;
    }

    // ボタン検知イベントを業務処理スレッドに引き渡す
    app_event_notify(status_pressed ? APEVT_BUTTON_1_PUSHED : APEVT_BUTTON_1_RELEASED);
}

static const struct device *initialize_button(const char *name, gpio_pin_t pin, gpio_flags_t flags, struct gpio_callback *callback, gpio_callback_handler_t handler)
{
    const struct device *button = device_get_binding(name);
    if (button == NULL) {
        LOG_ERR("Error: didn't find %s device", name);
        return NULL;
    }

    int ret = gpio_pin_configure(button, pin, flags);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d", ret, name, pin);
        return NULL;
    }

    ret = gpio_pin_interrupt_configure(button, pin, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", ret, name, pin);
        return NULL;
    }

    // ボタン押下時のコールバックを設定
    gpio_init_callback(callback, handler, BIT(pin));
    gpio_add_callback(button, callback);

    // ボタンの参照を戻す
#if LOG_BUTTON_INITIALIZED
    LOG_DBG("Set up button at %s pin %d", name, pin);
#endif
    return button;
}

//
// LED関連
//
static const struct device *m_led_0, *m_led_1, *m_led_2, *m_led_3;

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
#if LOG_BUTTON_INITIALIZED
    LOG_DBG("Set up LED at %s pin %d", name, pin);
#endif
    return led;
}

void app_board_initialize(void)
{
    // ボタンの初期化
    button_0 = initialize_button(SW0_GPIO_LABEL, SW0_GPIO_PIN, SW0_GPIO_FLAGS, &button_cb_0, button_pressed_0);
    button_1 = initialize_button(SW1_GPIO_LABEL, SW1_GPIO_PIN, SW1_GPIO_FLAGS, &button_cb_1, button_pressed_1);
    
    // LED0の初期化
    m_led_0 = initialize_led(LED0_GPIO_LABEL, LED0_GPIO_PIN, LED0_GPIO_FLAGS);
    m_led_1 = initialize_led(LED1_GPIO_LABEL, LED1_GPIO_PIN, LED1_GPIO_FLAGS);
    m_led_2 = initialize_led(LED2_GPIO_LABEL, LED2_GPIO_PIN, LED2_GPIO_FLAGS);
    m_led_3 = initialize_led(LED3_GPIO_LABEL, LED3_GPIO_PIN, LED3_GPIO_FLAGS);
}

void app_board_led_light(LED_COLOR led_color, bool led_on)
{
    // 業務で使用するLEDを点灯／消灯
    //   LED1=Yellow (Orange)
    //   LED2=Red
    //   LED3=Green
    //   LED4=Blue
    switch (led_color) {
        case LED_COLOR_YELLOW:
            gpio_pin_set(m_led_0, LED0_GPIO_PIN, led_on ? 1 : 0);
            break;
        case LED_COLOR_RED:
            gpio_pin_set(m_led_1, LED1_GPIO_PIN, led_on ? 1 : 0);
            break;
        case LED_COLOR_GREEN:
            gpio_pin_set(m_led_2, LED2_GPIO_PIN, led_on ? 1 : 0);
            break;
        case LED_COLOR_BLUE:
            gpio_pin_set(m_led_3, LED3_GPIO_PIN, led_on ? 1 : 0);
            break;
        default:
            break;
    }
}

//
// ディープスリープ（system off）状態に遷移
// --> ボタン押下でシステムが再始動
//
#include <hal/nrf_gpio.h>
#include <pm/pm.h>

void app_board_prepare_for_deep_sleep(void)
{
    // Configure to generate PORT event (wakeup) on button-1 press.
    nrf_gpio_cfg_input(SW0_GPIO_PIN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_sense_set(SW0_GPIO_PIN, NRF_GPIO_PIN_SENSE_LOW);

    printk("Entering system off; press BUTTON to restart... \n\n\r");
    pm_power_state_force((struct pm_state_info){PM_STATE_SOFT_OFF, 0, 0});
    k_sleep(K_MSEC(100));
}

//
// システムを再始動させる
// （RESETボタン押下と等価の処理）
//
void app_board_prepare_for_system_reset(void)
{
    sys_reboot(SYS_REBOOT_WARM);
}

//
// ブートローダーモードに遷移させる
//
#include <hal/nrf_power.h>

#define BOOTLOADER_DFU_GPREGRET         (0xB0)
#define BOOTLOADER_DFU_START_BIT_MASK   (0x01)
#define BOOTLOADER_DFU_START            (BOOTLOADER_DFU_GPREGRET | BOOTLOADER_DFU_START_BIT_MASK)

void app_board_prepare_for_bootloader_mode(void)
{
    // ブートローダーモードに遷移させるため、
    // GPREGRETレジスターにその旨の値を設定
    nrf_power_gpregret_set(NRF_POWER, BOOTLOADER_DFU_START);
}
