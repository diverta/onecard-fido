/* 
 * File:   ccid_oath_object.c
 * Author: makmorit
 *
 * Created on 2022/06/15, 13:36
 */
#include <string.h>
#include "ccid_oath.h"
#include "ccid_oath_account.h"
#include "ccid_oath_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug
#define LOG_ACCOUNT_EXIST_AND_SERIAL    false
#define LOG_ACCOUNT_READ                false
#define LOG_ACCOUNT_FETCH               false

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_object);
#endif

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// Flash ROM書込時のレスポンス制御
//
// APDU格納領域の参照を待避
static command_apdu_t  *m_capdu;
static response_apdu_t *m_rapdu;

void ccid_oath_object_resume_prepare(command_apdu_t *capdu, response_apdu_t *rapdu)
{
    // Flash ROM書込みが完了するまで、レスポンスを抑止
    ccid_apdu_response_set_pending(true);

    // APDU格納領域の参照を待避
    m_capdu = capdu;
    m_rapdu = rapdu;
}

void ccid_oath_object_resume_response(uint16_t sw)
{
    // レスポンス処理再開を指示
    m_rapdu->sw = sw;
    ccid_apdu_response_set_pending(false);
    ccid_apdu_resume_process(m_capdu, m_rapdu);
}

//
// アカウント登録
//
static uint8_t account_read_buff[160];
static uint8_t account_write_buff[160];

static bool write_account_object(uint8_t *write_buff, size_t write_size, uint16_t serial)
{
    // Flash ROMに登録
    //  Flash ROM更新後、
    //  ccid_oath_object_write_retry または
    //  ccid_oath_object_write_resume のいずれかが
    //  コールバックされます。
    size_t buffer_size = write_size;
    if (ccid_flash_oath_object_write(OATH_TAG_NAME, write_buff, buffer_size, true, serial) == false) {
        fido_log_error("OATH account write fail");
        return false;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)write_account_object;

    // 処理成功
    return true;
}

uint16_t ccid_oath_object_account_set(char *account_name, uint8_t account_name_size, char *secret, uint8_t secret_size, uint8_t property, uint8_t *challenge)
{
    // Flash ROMに登録するオブジェクトデータを生成
    // バイトイメージ（141バイト固定）
    //   0   : アカウント（64バイト）
    //   64  : アカウント長（1バイト）
    //   65  : アルゴリズム（1バイト）
    //   66  : OTP桁数（1バイト）
    //   67  : Secret（64バイト）
    //   131 : Secret長（1バイト）
    //   132 : オプション属性（1バイト）
    //   133 : Challenge（8バイト）
    uint8_t offset = 0;
    
    // アカウント
    memcpy(account_write_buff + offset, account_name, MAX_NAME_LEN);
    offset += MAX_NAME_LEN;
    account_write_buff[offset++] = account_name_size;

    // アルゴリズム、OTP桁数、Secret
    memcpy(account_write_buff + offset, secret, MAX_KEY_LEN);
    offset += MAX_KEY_LEN;
    account_write_buff[offset++] = secret_size;

    // オプション属性
    account_write_buff[offset++] = property;

    // Challenge
    memcpy(account_write_buff + offset, challenge, MAX_CHALLENGE_LEN);
    offset += MAX_CHALLENGE_LEN;

    //
    // 連番を設定
    //   exist==true時:  既存レコードに付与された連番
    //   exist==false時: 新規レコードに付与すべき連番
    //
    bool exist;
    uint16_t serial;
    if (ccid_flash_oath_object_find(OATH_TAG_NAME, (uint8_t *)account_name, account_name_size, account_read_buff, &exist, &serial) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

#if LOG_ACCOUNT_EXIST_AND_SERIAL
    fido_log_debug("account record(%s): exist=%d, serial=%d", log_strdup(account_name), exist, serial);
#endif

    // Flash ROMに登録
    if (write_account_object(account_write_buff, offset, serial) == false) {
        return SW_UNABLE_TO_PROCESS;
    }
    
    // 処理成功
    return SW_NO_ERROR;
}

uint16_t ccid_oath_object_account_delete(char *account_name, uint8_t account_name_size)
{
    //
    // Flash ROMから該当レコードを削除
    //
    bool exist;
    uint16_t serial;
    if (ccid_flash_oath_object_delete(OATH_TAG_NAME, (uint8_t *)account_name, account_name_size, account_read_buff, &exist, &serial) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

#if LOG_ACCOUNT_EXIST_AND_SERIAL
    fido_log_debug("account record(%s): exist=%d, serial=%d", log_strdup(account_name), exist, serial);
#endif

    // 該当レコードが無い場合はエラー
    if (exist == false) {
        return SW_DATA_INVALID;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_oath_object_account_delete;

    // 処理成功
    return SW_NO_ERROR;
}

uint16_t ccid_oath_object_delete_all(void)
{
    //
    // Flash ROMから全レコードを削除
    //
    if (ccid_flash_oath_object_delete_all() == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // Flash ROM書込み後のコールバック先を判定するため、関数参照を設定
    m_flash_func = (void *)ccid_oath_object_delete_all;

    // 処理成功
    return SW_NO_ERROR;
}

uint16_t ccid_oath_object_account_read(char *account_name, uint8_t account_name_size, char *secret, uint8_t *secret_size, uint8_t *property, uint8_t *challenge, bool *exist)
{
    //
    // Flash ROMから対象レコードを検索
    //
    uint16_t serial;
    if (ccid_flash_oath_object_find(OATH_TAG_NAME, (uint8_t *)account_name, account_name_size, account_read_buff, exist, &serial) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

#if LOG_ACCOUNT_READ
    // account_read_buff の内容（143バイト）：
    //  レコード長（4バイト）
    //  バイトイメージ（141バイト固定）
    //   0   : アカウント（64バイト）
    //   64  : アカウント長（1バイト）
    //   65  : アルゴリズム（1バイト）
    //   66  : OTP桁数（1バイト）
    //   67  : Secret（64バイト）
    //   131 : Secret長（1バイト）
    //   132 : オプション属性（1バイト）
    //   133 : Challenge（8バイト）
    fido_log_debug("account record(%s): exist=%d", log_strdup(account_name), *exist);
    fido_log_print_hexdump_debug(account_read_buff + 4 , 65 + 76, "record bytes");
#endif

    // 既存データがない場合はここで終了
    if (*exist == false) {
        return SW_NO_ERROR;
    }

    // 取得したデータを、引数の領域に格納
    uint8_t offset = 4 + MAX_NAME_LEN + 1;

    // アルゴリズム、OTP桁数、Secret
    if (secret != NULL) {
        memcpy(secret, account_read_buff + offset, MAX_KEY_LEN);
    }
    offset += MAX_KEY_LEN;
    if (secret_size != NULL) {
        *secret_size = account_read_buff[offset];
    }
    offset++;

    // オプション属性
    if (property != NULL) {
        *property = account_read_buff[offset];
    }
    offset++;

    // Challenge
    if (challenge != NULL) {
        memcpy(challenge, account_read_buff + offset, MAX_CHALLENGE_LEN);
    }
    offset += MAX_CHALLENGE_LEN;

    // 処理成功
    return SW_NO_ERROR;
}

//
// アカウントデータを読込
//
// 使用する関数の参照を保持
static int (*account_fetch_func)(uint8_t *data, size_t size);

static int fetch_account_data(const char *settings_key, void *account_buff, size_t account_buff_size)
{
    // データのサイズを取得
    (void)account_buff_size;
    uint32_t size32_t;
    memcpy(&size32_t, account_buff, sizeof(uint32_t));
    size_t size = (size_t)size32_t;
    
    // データの先頭アドレスを取得
    uint8_t *data = (uint8_t *)account_buff + 4;
    
#if LOG_ACCOUNT_FETCH
    // account_read_buff の内容（143バイト）：
    //  レコード長（4バイト）
    //  バイトイメージ（141バイト固定）
    //   0   : アカウント（64バイト）
    //   64  : アカウント長（1バイト）
    //   65  : アルゴリズム（1バイト）
    //   66  : OTP桁数（1バイト）
    //   67  : Secret（64バイト）
    //   131 : Secret長（1バイト）
    //   132 : オプション属性（1バイト）
    //   133 : Challenge（8バイト）
    fido_log_debug("account record(%s)", log_strdup(settings_key));
    fido_log_print_hexdump_debug(data, size, "record bytes");
#endif

    return (*account_fetch_func)(data, size);
}

uint16_t ccid_oath_object_account_fetch(int (*_fetch_func)(uint8_t *data, size_t size))
{
    // 例外抑止
    if (_fetch_func == NULL) {
        return SW_UNABLE_TO_PROCESS;
    }
    
    // 関数の参照を保持
    account_fetch_func = _fetch_func;
    
    // Flash ROMからレコードを読込
    if (ccid_flash_oath_object_fetch(OATH_TAG_NAME, fetch_account_data) == false) {
        return SW_UNABLE_TO_PROCESS;
    }

    // 処理成功
    return SW_NO_ERROR;
}

//
// Flash ROM書込み後のコールバック関数
//
void ccid_oath_object_write_retry(void)
{
    // リトライが必要な場合は
    // 呼び出し先に応じて、処理を再実行
    if (m_flash_func == write_account_object) {
        ccid_oath_account_retry();
    }
    if (m_flash_func == ccid_oath_object_account_delete) {
        ccid_oath_account_retry();
    }
    if (m_flash_func == ccid_oath_object_delete_all) {
        ccid_oath_account_retry();
    }
}

void ccid_oath_object_write_resume(bool success)
{
    // Flash ROM書込みが完了した場合は
    // 正常系の後続処理を実行
    if (m_flash_func == write_account_object) {
        ccid_oath_account_resume(success);
    }
    if (m_flash_func == ccid_oath_object_account_delete) {
        ccid_oath_account_resume(success);
    }
    if (m_flash_func == ccid_oath_object_delete_all) {
        ccid_oath_account_resume(success);
    }
}
