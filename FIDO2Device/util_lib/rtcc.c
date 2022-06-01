/* 
 * File:   rtcc.c
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:06
 */
#include "app_rtcc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(rtcc);
#endif

// 作業用領域
static char work_buf[32];

void rtcc_init(void)
{
    // RTCCの初期化
    if (app_rtcc_initialize() == false) {
        return;
    }

    fido_log_info("RTCC is available");
}
