/* 
 * File:   ccid_openpgp_object.c
 * Author: makmorit
 *
 * Created on 2021/02/11, 15:46
 */
#include "ccid_pin.h"
#include "ccid_openpgp.h"
#include "ccid_openpgp_pin.h"
#include "ccid_openpgp_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// テスト用
#define LOG_DEBUG_PIN_BUFFER    false

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

// APDU格納領域の参照を待避
static command_apdu_t  *m_capdu;
static response_apdu_t *m_rapdu;

void ccid_openpgp_object_resume_prepare(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // Flash ROM書込みが完了するまで、レスポンスを抑止
    ccid_apdu_response_set_pending(true);

    // APDU格納領域の参照を待避
    m_capdu = capdu;
    m_rapdu = rapdu;
}

void ccid_openpgp_object_resume_process(uint16_t sw)
{
    // レスポンス処理再開を指示
    m_rapdu->sw = sw;
    ccid_apdu_response_set_pending(false);
    ccid_apdu_resume_process(m_capdu, m_rapdu);
}

//
// PIN管理用
//
static uint8_t pin_read_buff[66];
static uint8_t pin_write_buff[66];

void ccid_openpgp_object_pin_clear(void)
{
    memset(pin_read_buff, 0, sizeof(pin_read_buff));
}

static uint16_t get_pin_obj_tag(PIN_TYPE type) 
{
    switch (type) {
        case OPGP_PIN_PW1:
            return TAG_OPGP_PW1;
        case OPGP_PIN_PW3:
            return TAG_OPGP_PW3;
        default:
            return TAG_OPGP_NONE;
    }
}

bool ccid_openpgp_object_pin_get(PIN_T *pin, uint8_t **pin_code, uint8_t *pin_size, uint8_t *retries)
{
    // オブジェクトデータをFlash ROMから読出し
    // バイトイメージ（最大66バイト）
    //   0      : PINリトライカウンター
    //   1      : PIN長
    //   2 - 65 : PIN（最大64バイト）
    bool is_exist = false;
    size_t pin_buffer_size;
    if (ccid_flash_object_read_by_tag(APPLET_OPENPGP, get_pin_obj_tag(pin->type), &is_exist, pin_read_buff, &pin_buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP PIN read fail: type=0x%02x", pin->type);
        return false;
    }
    if (is_exist == false) {
        // Flash ROMに登録されていない場合はデフォルトを設定
        pin_read_buff[0] = pin->default_retries;
        pin_read_buff[1] = strlen(pin->default_code);
        memcpy(pin_read_buff + 2, pin->default_code, strlen(pin->default_code));
        pin_buffer_size = pin_read_buff[1] + 2;
        fido_log_debug("OpenPGP PIN is not registered, use default: type=0x%02x", pin->type);
    }

#if LOG_DEBUG_PIN_BUFFER
    fido_log_debug("PIN object data read buffer (type=0x%02x): ", pin->type);
    fido_log_print_hexdump_debug(pin_read_buff, pin_buffer_size);
#endif

    // PINとリトライカウンターを戻す
    if (retries != NULL) {
        *retries = pin_read_buff[0];
    }
    if (pin_size != NULL) {
        *pin_size = pin_read_buff[1];
    }
    if (pin_code != NULL) {
        *pin_code = pin_read_buff + 2;
    }
    return true;
}

bool ccid_openpgp_object_pin_set(PIN_T *pin, uint8_t *pin_code, uint8_t pin_size, uint8_t retries)
{
    // Flash ROMに登録するオブジェクトデータを生成
    // バイトイメージ（最大66バイト）
    //   0      : PINリトライカウンター
    //   1      : PIN長
    //   2 - 65 : PIN（最大64バイト）
    pin_write_buff[0] = retries;
    pin_write_buff[1] = pin_size;
    memcpy(pin_write_buff + 2, pin_code, pin_size);

#if LOG_DEBUG_PIN_BUFFER
    fido_log_debug("PIN object data write buffer (type=0x%02x): ", pin->type);
    fido_log_print_hexdump_debug(pin_write_buff, pin_size + 2);
#endif

    // Flash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_object_write_retry または
    //  ccid_openpgp_object_write_resume のいずれかが
    //  コールバックされます。
    size_t pin_buffer_size = pin_size + 2;
    if (ccid_flash_object_write_by_tag(APPLET_OPENPGP, get_pin_obj_tag(pin->type), pin_write_buff, pin_buffer_size) == false) {
        fido_log_error("OpenPGP PIN write fail: type=0x%02x", pin->type);
        return false;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_openpgp_object_pin_set;

    // 処理成功
    return true;
}

//
// Flash ROM書込み後のコールバック関数
//
void ccid_openpgp_object_write_retry(void)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    // リトライが必要な場合は
    // 呼び出し先に応じて、処理を再実行
    if (m_flash_func == ccid_openpgp_object_pin_set) {
        ccid_openpgp_pin_retry();
    }
}

void ccid_openpgp_object_write_resume(bool success)
{
    ASSERT(m_capdu);
    ASSERT(m_rapdu);

    // Flash ROM書込みが完了した場合は
    // 正常系の後続処理を実行
    if (m_flash_func == ccid_openpgp_object_pin_set) {
        ccid_openpgp_pin_resume(success);
    }
}
