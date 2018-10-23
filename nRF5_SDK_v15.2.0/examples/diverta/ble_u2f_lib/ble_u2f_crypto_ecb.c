#include "sdk_common.h"

#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_flash.h"
#include "ble_u2f_crypto_ecb.h"
#include "ble_u2f_util.h"
#include "fds.h"

// for nrf_drv_rng_xxx
#include "nrf_drv_rng.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_crypto_ecb
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// AES ECBで使用する作業用エリア
#define ECB_BLOCK_LENGTH 16
static nrf_ecb_hal_data_t m_ecb_data;
static uint8_t block_cipher[ECB_BLOCK_LENGTH];

// AES ECBで使用する初期化ベクターとパスワード
uint8_t *m_initialization_vector;
uint8_t *m_password;

// Flash ROM書込み用データの一時格納領域
static fds_record_t        m_fds_record;
static uint32_t            m_random_vector[8];

// AES CFBモード
#define AES_CFB_MODE_ENCRYPTION 0
#define AES_CFB_MODE_DECRYPTION 1

// キーハンドル生成・格納用領域
// Register, Authenticateで共通使用
uint8_t keyhandle_base_buffer[64];
uint8_t keyhandle_buffer[64];


static bool write_random_vector(uint32_t *p_fds_record_buffer)
{
    ret_code_t ret;

    // 一時領域（確保済み）のアドレスを取得
    m_fds_record.data.p_data       = p_fds_record_buffer;
    m_fds_record.data.length_words = 8;
    m_fds_record.file_id           = U2F_AESKEYS_FILE_ID;
    m_fds_record.key               = U2F_AESKEYS_MODE_RECORD_KEY;

    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret = fds_record_find(U2F_AESKEYS_FILE_ID, U2F_AESKEYS_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // 既存のデータが存在する場合は上書き
        ret = fds_record_update(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("write_random_vector: fds_record_update returns 0x%02x ", ret);
            return false;
        }

    } else if (ret == FDS_ERR_NOT_FOUND) {
        // 既存のデータが存在しない場合は新規追加
        ret = fds_record_write(&record_desc, &m_fds_record);
        if (ret != FDS_SUCCESS && ret != FDS_ERR_NO_SPACE_IN_FLASH) {
            NRF_LOG_ERROR("write_random_vector: fds_record_write returns 0x%02x ", ret);
            return false;
        }

    } else {
        NRF_LOG_DEBUG("write_random_vector: fds_record_find returns 0x%02x ", ret);
        return false;
    }

    if (ret == FDS_ERR_NO_SPACE_IN_FLASH) {
        // 書込みができない場合、ガベージコレクションを実行
        // (fds_gcが実行される。NGであればエラー扱い)
        NRF_LOG_ERROR("write_random_vector: no space in flash, calling FDS GC ");
        if (ble_u2f_flash_force_fdc_gc() == false) {
            return false;
        }
    }

    return true;
}

bool ble_u2f_crypto_ecb_init(void)
{
    // 32バイトのランダムベクターを生成
    memset(m_random_vector, 0, sizeof(m_random_vector));
    uint32_t err_code = nrf_drv_rng_rand((uint8_t *)m_random_vector, 32);
    APP_ERROR_CHECK(err_code);

    // Flash ROMに書き出して保存
    if (write_random_vector(m_random_vector) == false) {
        return false;
    }

    NRF_LOG_DEBUG("Generated random vector for AES password ");
    return true;
}


static bool read_random_vector_record(fds_record_desc_t *record_desc, uint32_t *data_buffer)
{
	fds_flash_record_t flash_record;
	uint32_t *data;
    uint16_t  data_length;
    ret_code_t err_code;

    err_code = fds_record_open(record_desc, &flash_record);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("read_random_vector_record: fds_record_open returns 0x%02x ", err_code);
        return false;
    }

    data = (uint32_t *)flash_record.p_data;
    data_length = flash_record.p_header->length_words;
    memcpy(data_buffer, data, data_length * sizeof(uint32_t));

    err_code = fds_record_close(record_desc);
    if (err_code != FDS_SUCCESS) {
        NRF_LOG_ERROR("read_random_vector_record: fds_record_close returns 0x%02x ", err_code);
        return false;	
    }
    return true;
}

static bool read_random_vector(uint32_t *p_fds_record_buffer)
{
    // １レコード分読込
    fds_record_desc_t record_desc;
    fds_find_token_t  ftok = {0};
    ret_code_t ret = fds_record_find(U2F_AESKEYS_FILE_ID, U2F_AESKEYS_MODE_RECORD_KEY, &record_desc, &ftok);
    if (ret == FDS_SUCCESS) {
        // レコードが存在するときは領域にデータを格納
        if (read_random_vector_record(&record_desc, p_fds_record_buffer) == false) {
            // データ格納失敗時
            return false;
        }
    } else {
        // レコードが存在しないときや
        // その他エラー発生時
        return false;
    }
    return true;
}

static bool retrieve_ecb_keys()
{
    if (read_random_vector(m_random_vector) == false) {
        // Flash ROMにランダムベクターを格納したレコードが存在しない場合
        // 処理終了
        return false;
    }

    // Flash ROMレコードから取り出したランダムベクターを
    // 16バイトずつ分割し、初期化ベクター、パスワードに設定
    m_initialization_vector = (uint8_t *)m_random_vector;
    m_password = (uint8_t *)m_random_vector + ECB_BLOCK_LENGTH;
    return true;
}

static void calculate_block_cipher(uint8_t *p_cleartext, uint8_t *p_key) 
{
    // AES ECB構造体、ブロック暗号格納領域を初期化
    memset(&m_ecb_data, 0, sizeof(nrf_ecb_hal_data_t));
    memset(block_cipher, 0, sizeof(block_cipher));

    // 引数のcleartext, keyからブロック暗号(16バイト)を自動生成
    memcpy(m_ecb_data.key, p_key, SOC_ECB_KEY_LENGTH);
    memcpy(m_ecb_data.cleartext, p_cleartext, SOC_ECB_CLEARTEXT_LENGTH);
    uint32_t err_code = sd_ecb_block_encrypt(&m_ecb_data);
    APP_ERROR_CHECK(err_code);

    // ブロック暗号を、引数の領域にセット
    memcpy(block_cipher, m_ecb_data.ciphertext, SOC_ECB_CIPHERTEXT_LENGTH);
}

static void process_aes_cfb_crypto(uint8_t mode, uint8_t *packet, uint32_t packet_length, uint8_t *out_packet) 
{
    // AES ECB暗号を取得
    retrieve_ecb_keys();

    // 最初のブロック暗号生成時の入力には
    // 初期化ベクターを指定
    uint8_t *cleartext = m_initialization_vector;

    for (int i = 0; i < packet_length; i += ECB_BLOCK_LENGTH) {
        // ブロック暗号を生成して暗号化
        calculate_block_cipher(cleartext, m_password);
        for (int j = 0; j < ECB_BLOCK_LENGTH; j++) {
            out_packet[i + j] = packet[i + j] ^ block_cipher[j];
        }
        // 次回ブロック暗号生成時に入力となる領域を設定
        if (mode == AES_CFB_MODE_ENCRYPTION) {
            cleartext = out_packet + i;
        } else {
            cleartext = packet + i;
        }
    }
}

void ble_u2f_crypto_ecb_encrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet) 
{
    process_aes_cfb_crypto(AES_CFB_MODE_ENCRYPTION, packet, packet_length, out_packet);
}

void ble_u2f_crypto_ecb_decrypt(uint8_t *packet, uint32_t packet_length, uint8_t *out_packet) 
{
    process_aes_cfb_crypto(AES_CFB_MODE_DECRYPTION, packet, packet_length, out_packet);
}

void ble_u2f_crypto_ecb_generate_keyhandle(uint8_t *p_appid_hash, uint8_t *private_key_value, uint32_t private_key_length)
{
    // Register/Authenticateリクエストから取得した
    // appIdHash、秘密鍵(リトルエンディアン)を指定の領域に格納
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    memcpy(keyhandle_base_buffer, p_appid_hash, U2F_APPID_SIZE);
    memcpy(keyhandle_base_buffer + U2F_APPID_SIZE, private_key_value, private_key_length);

    // AES ECB暗号対象のバイト配列＆長さを指定し、
    // Cipher Feedback Modeによる暗号化を実行
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    uint16_t data_length = 64;
    ble_u2f_crypto_ecb_encrypt(keyhandle_base_buffer, data_length, keyhandle_buffer);
}


void ble_u2f_crypto_ecb_restore_keyhandle_base(uint8_t *keyhandle_value, uint32_t keyhandle_length)
{
    // Authenticateリクエストから取得した
    // キーハンドルを指定の領域に格納
    memset(keyhandle_buffer, 0, sizeof(keyhandle_buffer));
    memcpy(keyhandle_buffer, keyhandle_value, keyhandle_length);

    // Cipher Feedback Modeにより暗号化された
    // バイト配列を、同じ手法により復号化
    memset(keyhandle_base_buffer, 0, sizeof(keyhandle_base_buffer));
    uint16_t data_length = 64;
    ble_u2f_crypto_ecb_decrypt(keyhandle_buffer, data_length, keyhandle_base_buffer);
 }
