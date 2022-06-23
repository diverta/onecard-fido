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

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_object);
#endif

// Flash ROM書込み時に実行した関数の参照を保持
static void *m_flash_func = NULL;

//
// アカウント登録
//
static uint8_t account_read_buff[160];
static uint8_t account_write_buff[140];

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

uint16_t ccid_oath_object_account_set(char *account_name, char *secret, uint8_t property, uint8_t *challange)
{
    // Flash ROMに登録するオブジェクトデータを生成
    // バイトイメージ（139バイト固定）
    //   0   : アカウント（64バイト）
    //   64  : アルゴリズム（1バイト）
    //   65  : OTP桁数（1バイト）
    //   66  : Secret（64バイト）
    //   130 : オプション属性（1バイト）
    //   131 : Challenge（8バイト）
    uint8_t offset = 0;
    
    // アカウント
    memcpy(account_write_buff + offset, account_name, MAX_NAME_LEN);
    offset += MAX_NAME_LEN;

    // アルゴリズム、OTP桁数、Secret
    memcpy(account_write_buff + offset, secret, MAX_KEY_LEN);
    offset += MAX_KEY_LEN;

    // オプション属性
    account_write_buff[offset++] = property;

    // Challenge
    memcpy(account_write_buff + offset, challange, MAX_CHALLENGE_LEN);
    offset += MAX_CHALLENGE_LEN;

    //
    // 連番を設定
    //   exist==true時:  既存レコードに付与された連番
    //   exist==false時: 新規レコードに付与すべき連番
    //
    bool exist;
    uint16_t serial;
    if (ccid_flash_oath_object_find(OATH_TAG_NAME, account_name, MAX_NAME_LEN, account_read_buff, &exist, &serial) == false) {
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

uint16_t ccid_oath_object_account_delete(char *account_name)
{
    //
    // Flash ROMから該当レコードを削除
    //
    bool exist;
    uint16_t serial;
    if (ccid_flash_oath_object_delete(OATH_TAG_NAME, account_name, MAX_NAME_LEN, account_read_buff, &exist, &serial) == false) {
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
}
