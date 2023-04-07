/* 
 * File:   tiny_tft_base.c
 * Author: makmorit
 *
 * Created on 2023/04/07, 17:36
 */
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
