/* 
 * File:   ccid_piv_pin_update.c
 * Author: makmorit
 *
 * Created on 2020/11/10, 9:21
 */
#include <string.h>

#include "ccid_apdu.h"
#include "ccid_piv_object.h"
#include "ccid_piv_pin_auth.h"

// 一時読込領域
static uint8_t work_buf[PIN_DEFAULT_SIZE];

//
// PIN更新処理
//
static uint16_t update_pin(uint8_t pin_type, uint8_t *pin_buf)
{
    // PIN／リトライカウンターをセットで登録
    // （PINのリトライカウンターは、デフォルトに設定）
    if (ccid_piv_object_pin_set(pin_type, pin_buf, PIN_DEFAULT_RETRY_CNT) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 処理成功
    return SW_NO_ERROR;
}

uint16_t ccid_piv_pin_update(uint8_t pin_type, uint8_t *pin_buf) 
{
    uint16_t sw;
    switch (pin_type) {
        case TAG_PIV_PIN:
        case TAG_KEY_PUK:
            // PINを更新
            sw = update_pin(pin_type, pin_buf);
            break;
        default:
            // Not supported
            sw = SW_REFERENCE_DATA_NOT_FOUND;
            break;
    }
    return sw;
}

//
// リトライカウンター更新処理
//
static uint16_t update_pin_retries(uint8_t pin_type, uint8_t retries)
{
    // 登録されているPIN／リトライカウンターを取得
    // （登録されていない場合はデフォルトが戻ります）
    if (ccid_piv_object_pin_get(pin_type, work_buf, NULL) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // PIN／リトライカウンターをセットで登録
    if (ccid_piv_object_pin_set(pin_type, work_buf, retries) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 一時読込領域を初期化して終了
    memset(work_buf, 0, sizeof(work_buf));
    return SW_NO_ERROR;
}

uint16_t ccid_piv_pin_update_retries(uint8_t pin_type, uint8_t retries) 
{
    uint16_t sw;
    switch (pin_type) {
        case TAG_PIV_PIN:
        case TAG_KEY_PUK:
            // PINは保持し、現在リトライカウンターのみを更新
            sw = update_pin_retries(pin_type, retries);
            break;
        default:
            // Not supported
            sw = SW_REFERENCE_DATA_NOT_FOUND;
            break;
    }
    return sw;
}
