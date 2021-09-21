/* 
 * File:   fido_flash.c
 * Author: makmorit
 *
 * Created on 2021/09/20, 11:07
 */
//
// プラットフォーム非依存コード
//
#include "fido_flash_define.h"
#include "fido_ctap2_command.h"
#include "fido_maintenance_skcert.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_flash);
#endif

// for debug hex dump data
#define LOG_HEXDUMP_SKEY_CERT_DATA      false
#define LOG_HEXDUMP_PIN_CODE_HASH       false

//
//  鍵・証明書関連
//
// 鍵・証明書データ読込用の作業領域（固定長）
static uint32_t skey_cert_data[SKEY_CERT_WORD_NUM];

uint32_t *fido_flash_skey_cert_data(void)
{
    return skey_cert_data;
}

uint8_t *fido_flash_skey_data(void)
{
    // 秘密鍵格納領域の開始アドレスを取得
    uint32_t *skey_buffer = fido_flash_skey_cert_data();
    return (uint8_t *)skey_buffer;
}

uint8_t *fido_flash_cert_data(void)
{
    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = fido_flash_skey_cert_data() + SKEY_WORD_NUM + 1;
    return (uint8_t *)cert_buffer;
}

uint32_t fido_flash_cert_data_length(void)
{
    // 証明書データ格納領域の長さを取得
    uint32_t *cert_buffer = fido_flash_skey_cert_data() + SKEY_WORD_NUM;
    uint32_t cert_buffer_length = *cert_buffer;
    return cert_buffer_length;
}

bool fido_flash_skey_cert_read(void)
{
    // 確保領域は0xff（Flash ROM未書込状態）で初期化
    memset(skey_cert_data, 0xff, sizeof(uint32_t) * SKEY_CERT_WORD_NUM);

    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    APP_SETTINGS_KEY key = {FIDO_SKEY_CERT_FILE_ID, FIDO_SKEY_CERT_RECORD_KEY, false, 0};
    size_t size;
    if (app_settings_find(&key, (void *)skey_cert_data, &size) == false) {
        return false;
    }

#if LOG_HEXDUMP_SKEY_CERT_DATA
    LOG_HEXDUMP_INF((void *)skey_cert_data, size, "Attestation data");
#endif

    return true;
}

bool fido_flash_skey_cert_available(void)
{
    // 一時領域が初期状態であればunavailableと判定
    // (初期状態=確保領域の全ワードが0xffffffff)
    for (uint16_t i = 0; i < SKEY_CERT_WORD_NUM; i++) {
        if (skey_cert_data[i] != 0xffffffff) {
            return true;
        }
    }
    return false;
}

bool fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length)
{
    // 秘密鍵部（リクエストデータの先頭32バイト）を領域に格納
    uint32_t *securekey_buffer = fido_flash_skey_cert_data();
    memcpy(securekey_buffer, data, 32);

    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = securekey_buffer + SKEY_WORD_NUM;
    uint16_t  cert_length = length - 32;

    // 証明書データの格納に必要なワード数をチェックする
    uint32_t cert_buffer_length = (cert_length - 1) / 4 + 2;
    if (cert_buffer_length > CERT_WORD_NUM) {
        LOG_ERR("cert data words(%d) exceeds max words(%d) ",
            cert_buffer_length, CERT_WORD_NUM);
        return false;
    }
    
    // １ワード目に、証明書の当初バイト数を格納し、
    // ２ワード目以降から、証明書のデータを格納するようにする
    // (エンディアンは変換せずにそのまま格納)
    cert_buffer[0] = (uint32_t)cert_length;
    memcpy(cert_buffer + 1, data + 32, cert_length);
    return true;
}

bool fido_flash_skey_cert_write(void)
{
    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {FIDO_SKEY_CERT_FILE_ID, FIDO_SKEY_CERT_RECORD_KEY, false, 0};
    size_t size = SKEY_CERT_WORD_NUM * sizeof(uint32_t);

    if (app_settings_save(&key, (void *)skey_cert_data, size)) {
        // 書込み成功の場合は、管理用コマンドの処理を継続
        fido_maintenance_command_skey_cert_record_updated();
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

bool fido_flash_skey_cert_delete(void)
{
    // 秘密鍵／証明書をFlash ROM領域から削除
    APP_SETTINGS_KEY key = {FIDO_SKEY_CERT_FILE_ID, 0, false, 0};
    if (app_settings_delete_multi(&key)) {
        // 削除成功の場合は、管理用コマンドの処理を継続
        fido_maintenance_command_skey_cert_file_deleted();
        return true;

    } else {
        // 削除失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

//
//  AESパスワード関連
//
// Flash ROM書込み用データの一時格納領域
static uint32_t m_random_vector[8];

static bool read_random_vector(uint32_t *p_random_vector)
{
    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    APP_SETTINGS_KEY key = {FIDO_AESKEYS_FILE_ID, FIDO_AESKEYS_RECORD_KEY, false, 0};
    size_t size;
    if (app_settings_find(&key, (void *)p_random_vector, &size) == false) {
        return false;
    }

    // レコードが存在しない場合は false を戻す
    if (size == 0) {
        LOG_ERR("AES password not exist");
        return false;
    }
    
    return true;
}

uint8_t *fido_flash_password_get(void)
{
    if (read_random_vector(m_random_vector) == false) {
        // Flash ROMにランダムベクターを格納したレコードが存在しない場合
        // 処理終了
        return NULL;
    }

    // Flash ROMレコードから取り出したランダムベクターを
    // パスワードに設定
    return (uint8_t *)m_random_vector;
}

static bool write_random_vector(uint32_t *p_random_vector)
{
    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {FIDO_AESKEYS_FILE_ID, FIDO_AESKEYS_RECORD_KEY, false, 0};
    size_t size = 8 * sizeof(uint32_t);

    if (app_settings_save(&key, (void *)p_random_vector, size)) {
        // 書込み成功の場合は、管理用コマンドの処理を継続
        fido_maintenance_command_aes_password_record_updated();
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

bool fido_flash_password_set(uint8_t *random_vector)
{
    // 32バイトのランダムベクターを生成
    memcpy((uint8_t *)m_random_vector, random_vector, 32);

    // Flash ROMに書き出して保存
    return write_random_vector(m_random_vector);
}

//
//  トークンカウンター関連
//
bool fido_flash_token_counter_delete(void)
{
    // トークンカウンターをFlash ROM領域から削除
    APP_SETTINGS_KEY key = {FIDO_TOKEN_COUNTER_FILE_ID, 0, false, 0};
    if (app_settings_delete_multi(&key)) {
        // 削除成功の場合
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_token_counter_file_deleted();
        // 管理用コマンドの処理を実行
        fido_maintenance_command_token_counter_file_deleted();
        return true;

    } else {
        // 削除失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}
