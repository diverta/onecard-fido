/* 
 * File:   fido_flash.cpp
 * Author: makmorit
 *
 * Created on 2019/07/31, 14:28
 */
#include "mbed.h"
#include "nvstore.h"
//
// プラットフォーム非依存コード
//
#include "fido_ctap2_command.h"
#include "fido_maintenance.h"
#include "fido_u2f_command.h"

#include "fido_flash.h"
#include "fido_log.h"

//
// NVStore内での管理用キー
//  0: 共有情報
//  1: AESパスワード
//  2: 秘密鍵
//  3: 証明書
//  4: リトライカウンター
//  5-99: トークンカウンター
//
#define NVSTORE_KEY_CONTEXT 0
#define NVSTORE_KEY_AESPSWD 1
#define NVSTORE_KEY_PRIVKEY 2
#define NVSTORE_KEY_RETRCNT 3
#define NVSTORE_KEY_TOKNCNT 4

//
// データ読込用の作業領域
//
// 鍵・証明書データ読込用の作業領域（固定長）
static uint32_t skey_cert_data[SKEY_CERT_WORD_NUM];

// トークンカウンター読込用の作業領域
//  レコードサイズ = 17 ワード
//    レコードキー: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝Public Key Credential Sourceから生成されたSHA-256ハッシュ値
//      （実質、Webサイト＋ユーザー＋鍵の組み合わせでユニークとなるキー）
//    トークンカウンター: 1ワード（4バイト)
//    Webサイト情報: 8ワード（32バイト）
//      U2Fの場合＝appIdHash
//      CTAP2の場合＝rpIdHash
#define FIDO_TOKEN_COUNTER_RECORD_SIZE 17
static uint32_t m_token_counter_record_buffer[FIDO_TOKEN_COUNTER_RECORD_SIZE];
static uint32_t m_token_counter;

// トークンカウンター読込用の作業領域
//  レコードサイズ = 9 ワード
//    PINコードハッシュ: 8ワード（32バイト）
//    リトライカウンター: 1ワード（4バイト)
#define FIDO_PIN_RETRY_COUNTER_RECORD_SIZE  9
static uint32_t m_pin_store_hash_record[FIDO_PIN_RETRY_COUNTER_RECORD_SIZE];

// AESパスワード読込用の作業領域
#define AES_PASSWORD_WORD 8
#define AES_PASSWORD_SIZE 32
static uint32_t m_aes_password[AES_PASSWORD_WORD];

//
// 後続処理判定用のフラグ
//
static bool m_skey_cert_deleted;
static bool m_skey_cert_updated;
static bool m_token_counter_deleted;
static bool m_token_counter_updated;
static bool m_retry_counter_updated;
static bool m_aes_password_updated;

void fido_flash_init(void)
{
    NVStore &nvstore = NVStore::get_instance();
    int rc = nvstore.init();
    if (rc != NVSTORE_SUCCESS) {
        fido_log_error("fido_flash_init: nvstore.init returns %d", rc);
        return;
    }

    // フラグをクリア
    m_skey_cert_deleted = false;
    m_skey_cert_updated = false;
    m_token_counter_deleted = false;
    m_token_counter_updated = false;
    m_retry_counter_updated = false;
    m_aes_password_updated = false;
}

void fido_flash_do_process(void)
{
    if (m_skey_cert_deleted) {
        // レコード削除が完了した場合
        m_skey_cert_deleted = false;
        // 管理用コマンドの処理を実行
        fido_maintenance_command_skey_cert_file_deleted();
        return;
    }

    if (m_skey_cert_updated) {
        // レコード登録が完了した場合
        m_skey_cert_updated = false;
        // 管理用コマンドの処理を実行
        fido_maintenance_command_skey_cert_record_updated();
        return;
    }

    if (m_token_counter_deleted) {
        // レコード削除が完了した場合
        m_token_counter_deleted = false;
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_token_counter_file_deleted();
        // 管理用コマンドの処理を実行
        fido_maintenance_command_token_counter_file_deleted();
        return;
    }

    if (m_token_counter_updated) {
        // レコード登録が完了した場合
        m_token_counter_updated = false;
        // U2Fコマンドの処理を実行
        fido_u2f_command_token_counter_record_updated();
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_token_counter_record_updated();
        return;
    }

    if (m_retry_counter_updated) {
        // レコード登録が完了した場合
        m_retry_counter_updated = false;
        // CTAP2コマンドの処理を実行
        fido_ctap2_command_retry_counter_record_updated();
        return;
    }

    if (m_aes_password_updated) {
        // レコード登録が完了した場合
        m_aes_password_updated = false;
        // 管理用コマンドの処理を実行
        fido_maintenance_command_aes_password_record_updated();
        return;
    }
}

static uint16_t get_max_key_token_counter(void)
{
    NVStore &nvstore = NVStore::get_instance();
    return nvstore.get_max_keys();
}

static bool fido_flash_record_write(uint16_t key, void *buf, size_t size)
{
    // 該当キーのレコードを登録する
    NVStore &nvstore = NVStore::get_instance();
    int ret = nvstore.set(key, size, buf);
    if (ret != NVSTORE_SUCCESS) {
        fido_log_error("fido_flash_record_write: nvstore.set(%d) returns %d", key, ret);
        return false;
    }

    return true;
}

static bool fido_flash_record_read(uint16_t key, void *buf, size_t *size)
{
    // 該当キーのレコードを探す
    NVStore &nvstore = NVStore::get_instance();
    uint16_t actual_size;
    int ret = nvstore.get(key, *size, buf, actual_size);
    if (ret == NVSTORE_SUCCESS) {
        // レコードが存在するときはレコード長を格納
        *size = actual_size;

    } else if (ret == NVSTORE_NOT_FOUND) {
        // レコードが存在しないときはOKとし、
        // レコード長に０を設定
        *size = 0;

    } else {
        // それ以外の場合はエラー終了
        fido_log_error("fido_flash_record_read: nvstore.get(%d) returns %d", key, ret);
        return false;
    }
    
    return true;
}

static bool fido_flash_record_delete(uint16_t key)
{
    // 削除対象のレコードを探す
    NVStore &nvstore = NVStore::get_instance();
    uint16_t actual_size;
    int ret = nvstore.get_item_size(key, actual_size);
    if (ret == NVSTORE_NOT_FOUND) {
        return true;
    } else if (ret != NVSTORE_SUCCESS) {
        fido_log_error("fido_flash_record_delete: nvstore.get(%d) returns %d", key, ret);
        return false;
    }

    // レコードの削除を実行
    ret = nvstore.remove(key);
    if (ret != NVSTORE_SUCCESS) {
        fido_log_error("fido_flash_record_delete: nvstore.remove(%d) returns %d", key, ret);
        return false;
    }

    return true;
}

//
// C --> CPP 呼出用インターフェース
//
bool _fido_flash_get_stat_csv(uint8_t *stat_csv_data, size_t *stat_csv_size)
{
    int      ret;
    uint16_t actual_size;
    uint32_t largest_contig;
    uint8_t  corrupted = 0;
    
    // 格納領域を初期化
    memset(stat_csv_data, 0, *stat_csv_size);
    
    // サイズを取得
    NVStore &nvstore = NVStore::get_instance();
    size_t nvstore_size = nvstore.size();
    largest_contig = nvstore_size;

    // 全アイテムの使用領域と破損状況を取得
    uint16_t max_keys = nvstore.get_max_keys();
    for (uint16_t k = 0; k < max_keys; k++) {
        ret = nvstore.get_item_size(k, actual_size);
        if (ret == NVSTORE_SUCCESS) {
            largest_contig -= actual_size;
            fido_log_debug("_fido_flash_get_stat_csv: key(%d) actual_size(%d)", k, actual_size);
        }
        if (ret == NVSTORE_DATA_CORRUPT) {
            fido_log_error("_fido_flash_get_stat_csv: key(%d) data corrupted", k);
            corrupted = 1;
        }
    }
    
    // 各項目をCSV化し、引数のバッファに格納
    sprintf((char *)stat_csv_data, 
        "words_available=%d,largest_contig=%d,corruption=%d", 
        nvstore_size,
        largest_contig,
        corrupted);
    *stat_csv_size = strlen((char *)stat_csv_data);
    fido_log_debug("Flash ROM statistics csv created (%s)", (char *)stat_csv_data);
    return true;
}

bool _fido_flash_skey_cert_delete(void)
{
    // 秘密鍵／証明書をFlash ROM領域から削除
    if (!fido_flash_record_delete(NVSTORE_KEY_PRIVKEY)) {
        return false;
    }

    m_skey_cert_deleted = true;
    return true;
}

bool _fido_flash_skey_cert_write(void)
{
    // 既存のデータが存在する場合は上書き
    // 既存のデータが存在しない場合は新規追加
    size_t size = SKEY_CERT_WORD_NUM * sizeof(uint32_t);
    if (!fido_flash_record_write(NVSTORE_KEY_PRIVKEY, skey_cert_data, size)) {
        return false;
    }
    
    m_skey_cert_updated = true;
    return true;
}

bool _fido_flash_skey_cert_read(void)
{
    // 確保領域は0xff（Flash ROM未書込状態）で初期化
    size_t size = SKEY_CERT_WORD_NUM * sizeof(uint32_t);
    memset(skey_cert_data, 0xff, size);

    // １レコード分読込
    return fido_flash_record_read(NVSTORE_KEY_PRIVKEY, skey_cert_data, &size);
}

bool _fido_flash_skey_cert_available(void)
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

bool _fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length)
{
    // 秘密鍵部（リクエストデータの先頭32バイト）を領域に格納
    uint32_t *securekey_buffer = _fido_flash_skey_cert_data();
    memcpy(securekey_buffer, data, 32);

    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = securekey_buffer + SKEY_WORD_NUM;
    uint16_t  cert_length = length - 32;

    // 証明書データの格納に必要なワード数をチェックする
    uint32_t cert_buffer_length = (cert_length - 1) / 4 + 2;
    if (cert_buffer_length > CERT_WORD_NUM) {
        fido_log_error("cert data words(%d) exceeds max words(%d) ",
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

uint32_t *_fido_flash_skey_cert_data(void)
{
    return skey_cert_data;
}

uint8_t *_fido_flash_skey_data(void)
{
    // 秘密鍵格納領域の開始アドレスを取得
    uint32_t *skey_buffer = _fido_flash_skey_cert_data();
    return (uint8_t *)skey_buffer;
}

uint8_t *_fido_flash_cert_data(void)
{
    // 証明書データ格納領域の開始アドレスを取得
    uint32_t *cert_buffer = _fido_flash_skey_cert_data() + SKEY_WORD_NUM + 1;
    return (uint8_t *)cert_buffer;
}

uint32_t _fido_flash_cert_data_length(void)
{
    // 証明書データ格納領域の長さを取得
    uint32_t *cert_buffer = _fido_flash_skey_cert_data() + SKEY_WORD_NUM;
    uint32_t cert_buffer_length = *cert_buffer;
    return cert_buffer_length;
}
//
// トークンカウンター管理
//
bool _fido_flash_token_counter_delete(void)
{
    // リトライカウンターをFlash ROM領域から削除
    if (!fido_flash_record_delete(NVSTORE_KEY_RETRCNT)) {
        return false;
    }

    // トークンカウンターをFlash ROM領域から削除
    uint16_t max_key = get_max_key_token_counter();
    for (uint16_t k = NVSTORE_KEY_TOKNCNT; k < max_key; k++) {
        if (!fido_flash_record_delete(k)) {
            return false;
        }
    }

    m_token_counter_deleted = true;
    return true;
}

uint32_t _fido_flash_token_counter_value(void)
{
    // カウンターを取得して戻す
    return m_token_counter_record_buffer[8];
}

uint8_t *_fido_flash_token_counter_get_check_hash(void)
{
    // カウンターに紐づくチェック用ハッシュが
    // 格納されている先頭アドレスを戻す
    // バッファ先頭からのオフセットは９ワード分
    uint8_t *hash_for_check = (uint8_t *)(m_token_counter_record_buffer + 9);
    return hash_for_check;
}

static uint16_t token_counter_record_key;

static bool token_counter_record_find(uint8_t *p_appid_hash)
{
    int      ret;
    uint16_t actual_size;
    size_t   size = FIDO_TOKEN_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    NVStore &nvstore = NVStore::get_instance();
    
    // 割当キーをクリア
    token_counter_record_key = 0;
    
    // Flash ROMから既存データを走査
    uint16_t max_key = get_max_key_token_counter();
    for (uint16_t k = NVSTORE_KEY_TOKNCNT; k < max_key; k++) {
        ret = nvstore.get(k, size, m_token_counter_record_buffer, actual_size);
        if (ret == NVSTORE_SUCCESS) {
            // レコードが見つかった場合は
            // 同じappIdHashのレコードかどうか判定 (先頭32バイトを比較)
            if (strncmp((char *)p_appid_hash, (char *)m_token_counter_record_buffer, 32) == 0) {
                // 見つかったレコードのキーを割当て、
                // trueを戻す
                token_counter_record_key = k;
                return true;
            }
            
        } else if (ret == NVSTORE_NOT_FOUND && token_counter_record_key == 0) {
            // レコードが見つからない場合は新しいキーを割り当て
            // 次のレコードに移動
            token_counter_record_key = k;

        } else {
            fido_log_error("token_counter_record_find: nvstore.get(%d) returns %d", k, ret);
            token_counter_record_key = 0;
            break;
        }
    }
    
    // 見つからなかったのでfalseを戻す
    return false;
}

bool _fido_flash_token_counter_read(uint8_t *p_appid_hash)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_token_counter_record_bufferに読込む
    return token_counter_record_find(p_appid_hash);
}

bool _fido_flash_token_counter_write(uint8_t *p_appid_hash, uint32_t token_counter, uint8_t *p_hash_for_check)
{
    // Flash ROMから既存データを走査
    // 処理がNGの場合は終了
    if (!token_counter_record_find(p_appid_hash) && token_counter_record_key == 0) {
        return false;
    }
    
    // ユニークキーとなるappIdHash部 (8ワード)
    memcpy((uint8_t *)m_token_counter_record_buffer, p_appid_hash, 32);

    // トークンカウンター部 (1ワード)
    m_token_counter = token_counter;
    m_token_counter_record_buffer[8] = m_token_counter;

    // チェック用のIdHash部 (8ワード)
    // バッファ先頭からのオフセットは９ワード（36バイト）分
    memcpy((uint8_t *)m_token_counter_record_buffer + 36, p_hash_for_check, 32);

    // 既存のデータが存在する場合は上書き
    // 既存のデータが存在しない場合は新規追加
    size_t size = FIDO_TOKEN_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    if (!fido_flash_record_write(token_counter_record_key, m_token_counter_record_buffer, size)) {
        return false;
    }
    
    m_token_counter_updated = true;
    return true;
}
//
// リトライカウンター管理
//
static bool pin_code_hash_record_find(void)
{
    // 作業領域の初期化
    size_t size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    memset(m_pin_store_hash_record, 0, size);

    // Flash ROMからデータを検索
    return fido_flash_record_read(NVSTORE_KEY_RETRCNT, m_pin_store_hash_record, &size);
}

bool _fido_flash_client_pin_store_hash_read(void)
{
    // Flash ROMから既存データを読込み、
    // 既存データがあれば、データを
    // m_pin_store_hash_record に読込む
    return pin_code_hash_record_find();
}

uint8_t *_fido_flash_client_pin_store_pin_code_hash(void)
{
    // レコード領域の先頭アドレスを戻す
    return (uint8_t *)m_pin_store_hash_record;
}

uint32_t _fido_flash_client_pin_store_retry_counter(void)
{
    // カウンターを取得して戻す
    // （レコード領域先頭から９ワード目）
    return m_pin_store_hash_record[8];
}

bool _fido_flash_client_pin_store_hash_write(uint8_t *p_pin_code_hash, uint32_t retry_counter)
{
    // Flash ROMから既存データを走査
    pin_code_hash_record_find();
    
    // PINコードハッシュ部 (8ワード)
    // NULLが引き渡された場合は、更新しないものとする
    if (p_pin_code_hash != NULL) {
        memcpy((uint8_t *)m_pin_store_hash_record, p_pin_code_hash, 32);
    }

    // トークンカウンター部 (1ワード)
    m_pin_store_hash_record[8] = retry_counter;

    // 既存のデータが存在する場合は上書き
    // 既存のデータが存在しない場合は新規追加
    size_t size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    if (!fido_flash_record_write(NVSTORE_KEY_RETRCNT, m_pin_store_hash_record, size)) {
        return false;
    }
    
    m_retry_counter_updated = true;
    return true;
}

bool _fido_flash_client_pin_store_pin_code_exist(void)
{
    // PINコードをFlash ROMから読み出し
    if (_fido_flash_client_pin_store_hash_read() == false) {
        return false;
    }

    // PINコードハッシュがゼロ埋めされている場合は未登録と判断
    uint8_t *pin_code_hash = _fido_flash_client_pin_store_pin_code_hash();
    uint8_t  pin_code_hash_size = FIDO_PIN_RETRY_COUNTER_RECORD_SIZE * sizeof(uint32_t);
    uint8_t  i;
    for (i = 0; i < pin_code_hash_size; i++) {
        if (pin_code_hash[i] != 0) {
            return true;
        }
    }
    return false;
}
//
// AESパスワード管理
//
uint8_t *_fido_flash_password_get(void)
{
    // １レコード分読込
    size_t size = AES_PASSWORD_SIZE;
    if (!fido_flash_record_read(NVSTORE_KEY_AESPSWD, m_aes_password, &size)) {
        // Flash ROMにAESパスワードが存在しない場合
        // 処理終了
        return NULL;
    }

    // Flash ROMレコードから取り出したAESパスワードの
    // 格納領域を戻す
    return (uint8_t *)m_aes_password;
}

bool _fido_flash_password_set(uint8_t *random_vector)
{
    // 32バイトのランダムベクターをAESパスワードとして設定し、
    // ワーク領域にコピー
    size_t size = AES_PASSWORD_SIZE;
    memcpy((uint8_t *)m_aes_password, random_vector, size);

    // 既存のデータが存在する場合は上書き
    // 既存のデータが存在しない場合は新規追加
    if (!fido_flash_record_write(NVSTORE_KEY_AESPSWD, m_aes_password, size)) {
        return false;
    }

    m_aes_password_updated = true;
    return true;
}
