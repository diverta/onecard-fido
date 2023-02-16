/* 
 * File:   app_board.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 16:25
 */
#include <stdio.h>
#include <string.h>
#include <zephyr/types.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/time_units.h>

#include "app_board.h"
#include "app_event.h"

// ログ出力制御
#include <zephyr/logging/log.h>
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

bool app_board_get_version_info_csv(uint8_t *info_csv_data, size_t *info_csv_size)
{
    // 格納領域を初期化
    memset(info_csv_data, 0, *info_csv_size);

    // 各項目をCSV化し、引数のバッファに格納
    sprintf((char *)info_csv_data,
        "DEVICE_NAME=\"%s\",FW_REV=\"%s\",HW_REV=\"%s\"", 
            CONFIG_BT_DIS_MODEL, CONFIG_BT_DIS_FW_REV_STR, CONFIG_BT_DIS_HW_REV_STR);

    *info_csv_size = strlen((char *)info_csv_data);
    LOG_DBG("Application version info csv created (%d bytes)", *info_csv_size);
    return true;
}

//
// ボタン関連
//
static const struct gpio_dt_spec button_0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static const struct gpio_dt_spec button_1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw1), gpios, {0});
static struct gpio_callback button_cb_0, button_cb_1;
static bool button_press_enabled = false;

void app_board_button_press_enable(bool b)
{
    button_press_enabled = b;
}

static bool button_pressed(const struct device *dev, gpio_pin_t pin, int *status_pressed, uint32_t *time_pressed)
{
    if (button_press_enabled == false) {
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
    if (button_pressed(dev, button_0.pin, &status_pressed, &time_pressed) == false) {
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
    if (button_pressed(dev, button_1.pin, &status_pressed, &time_pressed) == false) {
        return;
    }

    // ボタン検知イベントを業務処理スレッドに引き渡す
    app_event_notify(status_pressed ? APEVT_BUTTON_1_PUSHED : APEVT_BUTTON_1_RELEASED);
}

static bool initialize_button(const struct gpio_dt_spec *button, struct gpio_callback *callback, gpio_callback_handler_t handler)
{
    if (device_is_ready(button->port) == false) {
        LOG_ERR("Error: didn't find %s device", button->port->name);
        return false;
    }

    int ret = gpio_pin_configure_dt(button, GPIO_INPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure %s pin %d", ret, button->port->name, button->pin);
        return false;
    }

    ret = gpio_pin_interrupt_configure_dt(button, GPIO_INT_EDGE_BOTH);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure interrupt on %s pin %d", ret, button->port->name, button->pin);
        return false;
    }

    // ボタン押下時のコールバックを設定
    gpio_init_callback(callback, handler, BIT(button->pin));
    gpio_add_callback(button->port, callback);

    // ボタンの参照を戻す
#if LOG_BUTTON_INITIALIZED
    LOG_DBG("Set up button at %s pin %d", button->port->name, button->pin);
#endif
    return true;
}

//
// LED関連
//
static struct gpio_dt_spec m_led_0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios, {0});
static struct gpio_dt_spec m_led_1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios, {0});
static struct gpio_dt_spec m_led_2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios, {0});
static struct gpio_dt_spec m_led_3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios, {0});

static bool initialize_led(struct gpio_dt_spec *led)
{
    if (device_is_ready(led->port) == false) {
        LOG_ERR("Didn't find LED device %s", led->port->name);
        return false;
    }

    int ret = gpio_pin_configure_dt(led, GPIO_OUTPUT);
    if (ret != 0) {
        LOG_ERR("Error %d: failed to configure LED device %s pin %d", ret, led->port->name, led->pin);
        return false;
    }

    // 最初は消灯しておく
    gpio_pin_set(led->port, led->pin, 0);

    // LED0の参照を戻す
#if LOG_BUTTON_INITIALIZED
    LOG_DBG("Set up LED at %s pin %d", name, pin);
#endif
    return true;
}

void app_board_initialize(void)
{
    // ボタンの初期化
    initialize_button(&button_0, &button_cb_0, button_pressed_0);
    initialize_button(&button_1, &button_cb_1, button_pressed_1);
    
    // LED0の初期化
    initialize_led(&m_led_0);
    initialize_led(&m_led_1);
    initialize_led(&m_led_2);
    initialize_led(&m_led_3);
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
            gpio_pin_set(m_led_0.port, m_led_0.pin, led_on ? 1 : 0);
            break;
        case LED_COLOR_RED:
            gpio_pin_set(m_led_1.port, m_led_1.pin, led_on ? 1 : 0);
            break;
        case LED_COLOR_GREEN:
            gpio_pin_set(m_led_2.port, m_led_2.pin, led_on ? 1 : 0);
            break;
        case LED_COLOR_BLUE:
            gpio_pin_set(m_led_3.port, m_led_3.pin, led_on ? 1 : 0);
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
#include <zephyr/pm/pm.h>

static const uint8_t cpu = 0;
static const struct pm_state_info si = {PM_STATE_SOFT_OFF, 0, 0};

void app_board_prepare_for_deep_sleep(void)
{
    // ポート番号（Port 0=0x00, Port 1=0x20）をピン番号に付加
#if LOG_BUTTON_INITIALIZED
    LOG_DBG("Set up button for deep sleep at %s pin %d", button_0.port->name, button_0.pin);
#endif
    uint32_t sw0_pin_number = button_0.pin;
    if (strcmp(button_0.port->name, "GPIO_1") == 0) {
        sw0_pin_number |= (0x1 << 5);
    }

    // Configure to generate PORT event (wakeup) on button-1 press.
    nrf_gpio_cfg_input(sw0_pin_number, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_sense_set(sw0_pin_number, NRF_GPIO_PIN_SENSE_LOW);

    printk("Entering system off; press BUTTON to restart... \n\n\r");
    pm_state_force(cpu, &si);
    k_sleep(K_MSEC(100));
}

//
// システムを再始動させる
// （RESETボタン押下と等価の処理）
//
void app_board_prepare_for_system_reset(void)
{
    printk("System will restart... \n\n\r");
    NVIC_SystemReset();
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
