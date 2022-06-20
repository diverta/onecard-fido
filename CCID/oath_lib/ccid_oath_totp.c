/* 
 * File:   ccid_oath_totp.c
 * Author: makmorit
 *
 * Created on 2022/06/21, 7:55
 */
#include "ccid_apdu.h"
#include "ccid_oath_define.h"
#include "fido_common.h"
#include "rtcc.h"

static uint8_t get_oath_type(uint8_t alg_byte)
{
    return (alg_byte & OATH_TYPE_MASK);
}

//
// TOTPカウンターを使用し、時刻同期を実行
//
uint16_t ccid_oath_totp_set_timestamp(uint8_t *secret, uint8_t *challange)
{
    // TOTPでない場合は、何もせず正常終了
    uint8_t alg = secret[0];
    uint8_t oath_type = get_oath_type(alg);
    if (oath_type != OATH_TYPE_TOTP){
        return SW_NO_ERROR;
    }

    // Challangeをカウンター（64ビット整数）に変換
    uint64_t counter = fido_get_uint64_from_bytes(challange);

    // カウンターが未設定の場合は、何もせず正常終了
    if (counter == 0) {
        return SW_NO_ERROR;
    }

    // カウンターをRTCCに設定
    if (rtcc_update_timestamp_by_unixtime((uint32_t)counter) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
}
