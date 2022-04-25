/* 
 * File:   fido_flash.c
 * Author: makmorit
 *
 * Created on 2021/09/20, 11:07
 */
#include <stdlib.h>
//
// プラットフォーム非依存コード
//
#include "fido_flash_define.h"
#include "fido_ctap2_command.h"
#include "fido_maintenance_skcert.h"
#include "fido_u2f_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_flash);
#endif

// for debug hex dump data
#define LOG_HEXDUMP_SKEY_CERT_DATA      false
#define LOG_DEBUG_TOKEN_COUNTER         false
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
    // 見つかった場合は skey_cert_data にデータをセット
    //   見つからなかった場合、skey_cert_data にデータはセットされないが、
    //   関数戻り値は true とします。
    APP_SETTINGS_KEY key = {FIDO_SKEY_CERT_FILE_ID, FIDO_SKEY_CERT_RECORD_KEY, false, 0};
    bool exist;
    size_t size;
    if (app_settings_find(&key, &exist, (void *)skey_cert_data, &size) == false) {
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
    bool exist;
    size_t size;
    if (app_settings_find(&key, &exist, (void *)p_random_vector, &size) == false) {
        return false;
    }

    // レコードが存在しない場合は false を戻す
    if (exist == false) {
        LOG_DBG("AES password not exist");
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
//  トークンカウンター管理用
//  レコードサイズ = 17 ワード
//    レコードキー: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝Public Key Credential Sourceから生成されたSHA-256ハッシュ値
//      （実質、Webサイト＋ユーザー＋鍵の組み合わせでユニークとなるキー）
//    トークンカウンター: 1ワード（4バイト)
//    Webサイト情報: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝rpIdHash
static uint32_t m_token_counter_record_buffer[FIDO_TOKEN_COUNTER_RECORD_SIZE];

uint32_t fido_flash_token_counter_value(void)
{
    // カウンターを取得して戻す
    // （レコード領域先頭から９ワード目）
    return m_token_counter_record_buffer[8];
}

uint8_t *fido_flash_token_counter_get_check_hash(void)
{
    // カウンターに紐づくチェック用ハッシュが
    // 格納されている先頭アドレスを戻す
    // バッファ先頭からのオフセットは９ワード分
    uint8_t *hash_for_check = (uint8_t *)(m_token_counter_record_buffer + 9);
    return hash_for_check;
}

// 同一キー判定用
static void *m_unique_key;

// レコード連番を保持
static uint16_t m_serial;
static uint16_t m_serial_max;

// 作業領域
static char work_buf[16];

static bool get_serial_from_settings_key(const char *settings_key, uint16_t *serial)
{
    // settings_key が14バイトでない場合は終了
    if (strlen(settings_key) != 14) {
        *serial = 0;
        return false;
    }

    // settings_keyから連番を取り出す
    // 先頭の11バイト目から4バイト
    //   settings_key: "BFFB/BFEB/nnnn"形式。先頭の"app/"は含まれない
    strncpy(work_buf, settings_key + 10, 4);

    // 連番を引数領域に設定
    int i = atoi(work_buf);
    *serial = (uint16_t)i;
    return true;
}

static bool cmpkey_token_counter_record(const char *settings_key, void *data, size_t size)
{
    // settings_keyから連番を取り出す
    uint16_t serial;
    (void)get_serial_from_settings_key(settings_key, &serial);

    // 最大の連番を更新
    if (serial > m_serial_max) {
        m_serial_max = serial;
    }

    // 同じキーのレコードかどうか判定 (先頭32バイトを比較)
    (void)size;
    if (memcmp(data, m_unique_key, 32) == 0) {
        // 同一キーレコードの連番を保持
        m_serial = serial;
        return true;

    } else {
        m_serial = 0;
        return false;
    }
}

static bool token_counter_record_find(uint8_t *p_unique_key)
{
    // 最大連番をゼロクリア
    m_serial_max = 0;

    // レコードキーの参照を保持
    m_unique_key = (void *)p_unique_key;

    // Flash ROMから既存データを走査
    APP_SETTINGS_KEY key = {FIDO_TOKEN_COUNTER_FILE_ID, FIDO_TOKEN_COUNTER_RECORD_KEY, false, 0};
    bool exist;
    size_t size;
    if (app_settings_search(&key, &exist, m_token_counter_record_buffer, &size, cmpkey_token_counter_record) == false) {
        return false;
    }

    // 既存データがあれば true
    return exist;
}

bool fido_flash_token_counter_read(uint8_t *p_unique_key)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_token_counter_record_bufferに読込む
    bool ret = token_counter_record_find(p_unique_key);

#if LOG_DEBUG_TOKEN_COUNTER
    if (ret) {
        LOG_INF("Token counter record found (serial=%d, counter=%d)", m_serial, fido_flash_token_counter_value());
    } else {
        LOG_INF("Token counter record not found");
    }
#endif

    return ret;
}

bool fido_flash_token_counter_write(uint8_t *p_unique_key, uint32_t token_counter, uint8_t *p_rpid_hash)
{
    // Flash ROMから既存データを走査し、
    // 更新または新規追加のためのレコード連番を取得
    bool found = token_counter_record_find(p_unique_key);

    // ユニークキー部 (8ワード)
    memcpy((uint8_t *)m_token_counter_record_buffer, p_unique_key, 32);

    // トークンカウンター部 (1ワード)
    m_token_counter_record_buffer[8] = token_counter;

    // rpIdHash部 (8ワード)
    // バッファ先頭からのオフセットは９ワード（36バイト）分
    if (p_rpid_hash != NULL) {
        memcpy((uint8_t *)m_token_counter_record_buffer + 36, p_rpid_hash, 32);
    }

    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {FIDO_TOKEN_COUNTER_FILE_ID, FIDO_TOKEN_COUNTER_RECORD_KEY, true, 0};
    size_t size = FIDO_TOKEN_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    if (found) {
        // 既存のデータが存在する場合は上書き
        key.serial = m_serial;

    } else {
        // 既存のデータが存在しない場合は新規追加
        key.serial = m_serial_max + 1;
    }

#if LOG_DEBUG_TOKEN_COUNTER
    LOG_INF("Token counter record %s (serial=%d, counter=%d)", found ? "updated" : "created", key.serial, token_counter);
#endif
    
    // Flash ROMに書込
    if (app_settings_save(&key, (void *)m_token_counter_record_buffer, size)) {
        // 書込み成功の場合は、U2F／CTAP2コマンドの処理を継続
        fido_u2f_command_token_counter_record_updated();
        fido_ctap2_command_token_counter_record_updated();
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        LOG_ERR("Token counter record %s fail", found ? "update" : "create");
        return false;
    }
}

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

//
// リトライカウンター関連
//
// PINリトライカウンター管理用
//   レコードサイズ = 9 ワード
//     PINコードハッシュ: 8ワード（32バイト）
//     リトライカウンター: 1ワード（4バイト)
static uint32_t m_pin_store_hash_record[FIDO_PIN_RETRY_COUNTER_RECORD_SIZE];

static bool pin_code_hash_record_find(void)
{
    // 作業領域の初期化
    size_t size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    memset(m_pin_store_hash_record, 0, size);

    // Flash ROMから既存データを検索し、
    // 見つかった場合は true を戻す
    APP_SETTINGS_KEY key = {FIDO_PIN_RETRY_COUNTER_FILE_ID, FIDO_PIN_RETRY_COUNTER_RECORD_KEY, false, 0};
    bool exist;
    if (app_settings_find(&key, &exist, (void *)m_pin_store_hash_record, &size) == false) {
        return false;
    }

#if LOG_HEXDUMP_PIN_CODE_HASH
    LOG_HEXDUMP_INF((void *)m_pin_store_hash_record, size, "pin store hash record");
#endif

    // 既存データがあれば true
    return exist;
}

bool fido_flash_client_pin_store_hash_read(void)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_pin_store_hash_record に読込む
    return pin_code_hash_record_find();
}

uint8_t *fido_flash_client_pin_store_pin_code_hash(void)
{
    // レコード領域の先頭アドレスを戻す
    return (uint8_t *)m_pin_store_hash_record;
}

uint32_t fido_flash_client_pin_store_retry_counter(void)
{
    // カウンターを取得して戻す
    // （レコード領域先頭から９ワード目）
    return m_pin_store_hash_record[8];
}

bool fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter)
{
    // PINコードハッシュ部 (8ワード)
    // NULLが引き渡された場合は、更新しないものとする
    if (p_pin_code_hash != NULL) {
        memcpy((uint8_t *)m_pin_store_hash_record, p_pin_code_hash, SHA_256_HASH_SIZE);
    }

    // トークンカウンター部 (1ワード)
    m_pin_store_hash_record[8] = retry_counter;

    // Flash ROMに書込むキー／サイズを設定
    APP_SETTINGS_KEY key = {FIDO_PIN_RETRY_COUNTER_FILE_ID, FIDO_PIN_RETRY_COUNTER_RECORD_KEY, false, 0};
    size_t size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    if (app_settings_save(&key, (void *)m_pin_store_hash_record, size)) {
        // 書込み成功の場合は、CTAP2コマンドの処理を継続
        fido_ctap2_command_retry_counter_record_updated();
        return true;

    } else {
        // 書込み失敗の場合は、呼び出し元に制御を戻す
        return false;
    }
}

bool fido_flash_client_pin_store_pin_code_exist(void)
{
    // PINコードをFlash ROMから読み出し
    if (fido_flash_client_pin_store_hash_read() == false) {
        return false;
    }

    // PINコードハッシュがゼロ埋めされている場合は未登録と判断
    uint8_t *pin_code_hash = fido_flash_client_pin_store_pin_code_hash();
    uint8_t  pin_code_hash_size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * 4;
    uint8_t  i;
    for (i = 0; i < pin_code_hash_size; i++) {
        if (pin_code_hash[i] != 0) {
            return true;
        }
    }
    return false;
}
