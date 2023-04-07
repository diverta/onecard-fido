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
