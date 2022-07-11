/* 
 * File:   ccid_oath_calculate.c
 * Author: makmorit
 *
 * Created on 2022/07/11, 17:03
 */
#include "ccid_oath.h"
#include "ccid_oath_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_calculate);
#endif

uint16_t ccid_oath_calculate(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    //
    // アカウント名を抽出
    //
    uint8_t offset = 0;
    if (offset + 1 >= capdu->lc) {
        return(SW_WRONG_LENGTH);
    }
    if (capdu->data[offset++] != OATH_TAG_NAME) {
        return SW_WRONG_DATA;
    }
    uint8_t name_len = capdu->data[offset++];
    if (name_len > MAX_NAME_LEN || name_len == 0) {
        return SW_WRONG_DATA;
    }
    uint8_t name_offset = offset;
    offset += name_len;
    if (offset > capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    return SW_NO_ERROR;
}
