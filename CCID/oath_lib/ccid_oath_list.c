/* 
 * File:   ccid_oath_list.c
 * Author: makmorit
 *
 * Created on 2022/08/25, 16:12
 */
#include "ccid_oath.h"
#include "ccid_oath_list.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_list);
#endif

uint16_t ccid_oath_list(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return (SW_WRONG_P1P2);
    }

    // TODO: 仮の実装です。
    fido_log_info("OATH account list get");
    return SW_NO_ERROR;
}
