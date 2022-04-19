/* 
 * File:   ccid_openpgp_object.c
 * Author: makmorit
 *
 * Created on 2021/02/11, 15:46
 */
#include "ccid_pin.h"
#include "ccid_openpgp.h"
#include "ccid_openpgp_data.h"
#include "ccid_openpgp_object.h"
#include "ccid_openpgp_pin.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_openpgp_object);
#endif

// テスト用
#define LOG_DEBUG_PIN_BUFFER    false
#define LOG_DEBUG_OBJ_BUFFER    false

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
        case OPGP_PIN_RC:
            return TAG_OPGP_RC;
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
    if (ccid_flash_openpgp_object_read(APPLET_OPENPGP, get_pin_obj_tag(pin->type), &is_exist, pin_read_buff, &pin_buffer_size) == false) {
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
    if (ccid_flash_openpgp_object_write(APPLET_OPENPGP, get_pin_obj_tag(pin->type), pin_write_buff, pin_buffer_size) == false) {
        fido_log_error("OpenPGP PIN write fail: type=0x%02x", pin->type);
        return false;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_openpgp_object_pin_set;

    // 処理成功
    return true;
}

//
// OpenPGPデータオブジェクト管理用
//   最大データ長は仮設定です。
//
static uint8_t obj_read_buffer[512];
static uint8_t obj_write_buff[512];

bool ccid_openpgp_object_data_get(uint16_t obj_tag, uint8_t **obj_data, size_t *obj_size)
{
    // オブジェクトデータをFlash ROMから読出し
    bool is_exist = false;
    size_t buffer_size;
    if (ccid_flash_openpgp_object_read(APPLET_OPENPGP, obj_tag, &is_exist, obj_read_buffer, &buffer_size) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP data object read fail: tag=0x%04x", obj_tag);
        return false;
    }

#if LOG_DEBUG_OBJ_BUFFER
    if (is_exist) {
        fido_log_debug("OpenPGP data object read buffer (tag=0x%04x): ", obj_tag);
        fido_log_print_hexdump_debug(obj_read_buffer, buffer_size);
    }
#endif

    // データ格納位置およびデータ長を戻す
    if (obj_size != NULL) {
        *obj_size = buffer_size;
    }
    if (obj_data != NULL) {
        *obj_data = obj_read_buffer;
    }
    return true;
}

bool ccid_openpgp_object_data_set(uint16_t obj_tag, uint8_t *obj_data, size_t obj_size)
{
    // Flash ROMに登録するオブジェクトデータを生成
    memcpy(obj_write_buff, obj_data, obj_size);

#if LOG_DEBUG_OBJ_BUFFER
    fido_log_debug("OpenPGP data object write buffer (tag=0x%04x): ", obj_tag);
    fido_log_print_hexdump_debug(obj_write_buff, obj_size);
#endif

    // Flash ROMに登録
    //  Flash ROM更新後、
    //  ccid_openpgp_object_write_retry または
    //  ccid_openpgp_object_write_resume のいずれかが
    //  コールバックされます。
    if (ccid_flash_openpgp_object_write(APPLET_OPENPGP, obj_tag, obj_write_buff, obj_size) == false) {
        fido_log_error("OpenPGP object data write fail: tag=0x%04x", obj_tag);
        return false;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_openpgp_object_data_set;

    // 処理成功
    return true;
}

bool ccid_openpgp_object_data_delete_all(void)
{
    // Flash ROMから、全てのオブジェクトデータを削除
    //  Flash ROM更新後、
    //  ccid_openpgp_object_write_retry または
    //  ccid_openpgp_object_write_resume のいずれかが
    //  コールバックされます。
    if (ccid_flash_openpgp_object_delete_all(APPLET_OPENPGP) == false) {
        fido_log_error("All OpenPGP object data delete fail");
        return false;
    }
    
    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_openpgp_object_data_delete_all;

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
    if (m_flash_func == ccid_openpgp_object_data_set) {
        ccid_openpgp_data_retry();
    }
    if (m_flash_func == ccid_openpgp_object_data_delete_all) {
        ccid_openpgp_data_retry();
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
    if (m_flash_func == ccid_openpgp_object_data_set) {
        ccid_openpgp_data_resume(success);
    }
    if (m_flash_func == ccid_openpgp_object_data_delete_all) {
        ccid_openpgp_data_resume(success);
    }
}
