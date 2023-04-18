/* 
 * File:   tiny_tft_base.c
 * Author: makmorit
 *
 * Created on 2023/04/07, 17:36
 */
#include <zephyr/types.h>
#include <zephyr/kernel.h>

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// プラットフォーム依存コード
#include "app_tiny_tft.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(tiny_tft_base);
#endif

// プラットフォーム非依存コード
#include "tiny_tft_define.h"

//
// モジュール利用の可否照会
//
bool tiny_tft_is_available(void)
{
#ifdef CONFIG_USE_TINY_TFT
    return true;
#else
    return false;
#endif
}

void tiny_tft_base_start_reset(void)
{
    app_tiny_tft_set_rst(LOW);
}

void tiny_tft_base_end_reset(void)
{
    app_tiny_tft_set_rst(HIGH);
}

void tiny_tft_base_start_write(void)
{
    app_tiny_tft_set_c_s(LOW);
}

void tiny_tft_base_end_write(void)
{
    app_tiny_tft_set_c_s(HIGH);
}

void tiny_tft_base_delay_ms(uint32_t ms)
{
    k_sleep(K_MSEC(ms));
}

void tiny_tft_base_init(void)
{
    // Initialize spi config (SPI data clock frequency)
    app_tiny_tft_initialize(4000000);

    // Init basic control pins common to all connection types
    app_tiny_tft_set_c_s(HIGH);
    app_tiny_tft_set_d_c(HIGH);
}

//
// データ転送関連
//
static uint8_t work_buf[16];

bool tiny_tft_base_write_byte(uint8_t b)
{
    // １バイトを転送
    work_buf[0] = b;
    return app_tiny_tft_write(work_buf, 1);
}

bool tiny_tft_base_write_dword(uint32_t l)
{
    // ４バイトを転送
    work_buf[0] = l >> 24;
    work_buf[1] = l >> 16;
    work_buf[2] = l >> 8;
    work_buf[3] = l;
    return app_tiny_tft_write(work_buf, 4);
}

bool tiny_tft_base_write_command(uint8_t command_byte) 
{
    // Send the command byte
    app_tiny_tft_set_d_c(LOW);
    if (tiny_tft_base_write_byte(command_byte) == false) {
        return false;
    }
    app_tiny_tft_set_d_c(HIGH);
    return true;
}

bool tiny_tft_base_write_data(uint8_t command_byte, uint8_t *data_bytes, uint8_t data_size) 
{
    // Send the command byte
    app_tiny_tft_set_d_c(LOW);
    if (tiny_tft_base_write_byte(command_byte) == false) {
        return false;
    }

    // Send the data bytes
    app_tiny_tft_set_d_c(HIGH);
    if (data_size > 0) {
        if (app_tiny_tft_write(data_bytes, data_size) == false) {
            return false;
        }
    }
    return true;
}

void tiny_tft_base_backlight_on(void)
{
    app_tiny_tft_set_led(LOW);
}

void tiny_tft_base_backlight_off(void)
{
    app_tiny_tft_set_led(HIGH);
}
