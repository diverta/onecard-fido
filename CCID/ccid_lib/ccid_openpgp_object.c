/* 
 * File:   ccid_openpgp_object.c
 * Author: makmorit
 *
 * Created on 2021/02/11, 15:46
 */
#include "ccid_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// PIN管理用
//
static uint8_t pin_buffer[256];

void ccid_openpgp_object_pin_clear(void)
{
    memset(pin_buffer, 0, sizeof(pin_buffer));
}

bool ccid_openpgp_object_pin_get(PIN_T *pin, uint8_t **pin_code, uint8_t *pin_size, uint8_t *retries)
{

    // オブジェクトデータをFlash ROMから読出し
    // バイトイメージ（最大66バイト）
    //   0      : PINリトライカウンター
    //   1      : PIN長
    //   2 - 65 : PIN（最大64バイト）
    bool is_exist = false;
#if SAMPLE
    size_t obj_size = 0;
    if (ccid_flash_piv_object_data_read(pin->type, pin_buffer, &obj_size, &is_exist) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP PIN read fail: type=%02d", pin->type);
        return false;
    }
#endif
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        pin_buffer[0] = pin->default_retries;
        pin_buffer[1] = strlen(pin->default_code);
        memcpy(pin_buffer + 2, pin->default_code, strlen(pin->default_code));
        fido_log_debug("OpenPGP PIN is not registered, use default: type=%02d", pin->type);
    }

#if LOG_DEBUG_PIN_BUFFER
    fido_log_debug("PIN object data read buffer (type=%2d): ", pin->type);
    fido_log_print_hexdump_debug(pin_buffer, sizeof(pin_buffer));
#endif

    // PINとリトライカウンターを戻す
    if (retries != NULL) {
        *retries = pin_buffer[0];
    }
    if (pin_size != NULL) {
        *pin_size = pin_buffer[1];
    }
    if (pin_code != NULL) {
        *pin_code = pin_buffer + 2;
    }
    return true;
}
