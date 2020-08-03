/* 
 * File:   ccid_ykpiv.c
 * Author: makmorit
 *
 * Created on 2020/08/03, 12:16
 */
#include <stdlib.h>
#include <string.h>

#include "ccid.h"
#include "ccid_piv.h"
#include "ccid_piv_object.h"
#include "ccid_ykpiv.h"

uint16_t ccid_ykpiv_ins_get_version(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // バージョン文字列 "xx.xx.xx" を分割
    uint8_t v[] = {0x00, 0x00, 0x00};
#ifdef FW_REV
    char *version_str = FW_REV;
    char *tp = strtok(version_str, ".");
    for (int i = 0; tp != NULL; i++) {
        v[i] = atoi(tp);
        tp = strtok(NULL, ".");
    }    
#endif
    
    // レスポンスデータを編集
    uint8_t *rdata = rapdu->data;
    rdata[0] = v[0];
    rdata[1] = v[1];
    rdata[2] = v[2];
    rapdu->len = 3;

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t ccid_ykpiv_ins_get_serial(command_apdu_t *capdu, response_apdu_t *rapdu) 
{
    // パラメーターのチェック
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }
    if (capdu->lc != 0) {
        return SW_WRONG_LENGTH;
    }

    // ファイルの内容を送信APDUデータに格納
    uint8_t *rdata = rapdu->data;
    size_t size = ccid_response_apdu_size_max();
    if (ccid_piv_object_sn_get(rdata, &size) == false) {
        return SW_FILE_NOT_FOUND;
    }
    rapdu->len = (uint16_t)size;

    // 正常終了
    return SW_NO_ERROR;
}
