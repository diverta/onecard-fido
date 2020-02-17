/* 
 * File:   fido_cryptoauth.c
 * Author: makmorit
 *
 * Created on 2019/11/25, 14:43
 */
//
// プラットフォーム非依存コード
//
#include "fido_cryptoauth.h"
#include "fido_cryptoauth_setup.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "atca_iface.h"
#include "cryptoauthlib.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_CONFIG false
#define LOG_HEXDUMP_DEBUG_PUBKEY false
#define LOG_HEXDUMP_DEBUG_SIGN   false
#define ATCAB_VERIFY_EXTERN      false
#define LOG_HEXDUMP_HMAC256_KEY  false
#define LOG_HEXDUMP_DEBUG_SSKEY  false

// 設定情報を保持
static ATCAIfaceCfg m_iface_config;
static uint8_t ateccx08a_config_bytes[ATCA_ECC_CONFIG_SIZE];

// 初期化処理実行済みフラグ
static bool atcab_init_done = false;

// シリアルナンバーを保持
static uint8_t cryptoauth_serial_num[ATCA_SERIAL_NUM_SIZE];

// 公開鍵を保持
static uint8_t public_key_raw_data[ATCA_PUB_KEY_SIZE];

// 秘密鍵を保持（atcab_priv_write で使用）
static uint8_t private_key_buffer[4 + ATCA_PRIV_KEY_SIZE];

// ランダムベクターを保持
static uint8_t m_random_vector[RANDOM_NUM_SIZE];

static bool get_cryptoauth_serial_num(void)
{
    ATCA_STATUS status = atcab_read_serial_number(cryptoauth_serial_num);
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_serial_no: atcab_read_serial_number() failed with ret=0x%02x", status);
        return false;
    }
#if LOG_HEXDUMP_DEBUG_CONFIG
    fido_log_debug("Serial number:");
    fido_log_print_hexdump_debug(cryptoauth_serial_num, ATCA_SERIAL_NUM_SIZE);
#endif
    return true;
}

bool fido_cryptoauth_get_config_bytes(void)
{
    ATCA_STATUS status = atcab_read_config_zone(ateccx08a_config_bytes);
    if (status != ATCA_SUCCESS) {
        fido_log_error("get_cryptoauth_config_bytes: atcab_read_config_zone() failed with ret=0x%02x", status);
        return false;
    }
#if LOG_HEXDUMP_DEBUG_CONFIG
    fido_log_debug("Config zone data (128 bytes):");
    fido_log_print_hexdump_debug(ateccx08a_config_bytes,      64);
    fido_log_print_hexdump_debug(ateccx08a_config_bytes + 64, 64);
#endif
    return true;
}

bool fido_cryptoauth_init(void)
{
    if (atcab_init_done) {
        // 初期化処理が実行済みの場合は終了
        return true;
    }
    fido_log_info("fido_cryptoauth_init start");

    // デバイス設定は、ライブラリーのデフォルトを採用
    ATCAIfaceCfg *p_cfg = &m_iface_config;
    *p_cfg = cfg_ateccx08a_i2c_default;

    // デバイスの初期化
    ATCA_STATUS status = atcab_init(p_cfg);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_init failed: atcab_init() returns 0x%02x", status);
        return false;
    }

    // シリアル番号を取得
    if (get_cryptoauth_serial_num() == false) {
        return false;
    }

    // Configがデフォルトのままであれば変更を行う
    if (fido_cryptoauth_setup_config() == false) {
        return false;
    }

    // 初期化処理は実行済み
    atcab_init_done = true;
    fido_log_info("fido_cryptoauth_init success");
    return true;
}

void fido_cryptoauth_release(void)
{
    if (atcab_init_done) {
        // デバイスを解放
        atcab_release();
        atcab_init_done = false;
        fido_log_info("fido_cryptoauth_release done");
    }
}

bool fido_cryptoauth_keypair_generate(uint16_t key_id)
{
    // 初期化
    memset(public_key_raw_data, 0x00, sizeof(public_key_raw_data));
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    ATCA_STATUS status = atcab_genkey(key_id, public_key_raw_data);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_keypair_generate failed: atcab_genkey(%d) returns 0x%02x", 
            key_id, status);
        return false;

    } else {
        fido_log_debug("fido_cryptoauth_keypair_generate success (key_id=%d)", key_id);
        return true;
    }
}

uint8_t *fido_cryptoauth_keypair_public_key(uint16_t key_id)
{
    // 初期化
    memset(public_key_raw_data, 0x00, sizeof(public_key_raw_data));
    if (fido_cryptoauth_init()== false) {
        return public_key_raw_data;
    }

    ATCA_STATUS status = atcab_get_pubkey(key_id, public_key_raw_data);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_keypair_public_key failed: atcab_get_pubkey(%d) returns 0x%02x", 
            key_id, status);

#if LOG_HEXDUMP_DEBUG_PUBKEY
    } else {
        fido_log_debug("fido_cryptoauth_keypair_public_key (key_id=%d):", key_id);
        fido_log_print_hexdump_debug(public_key_raw_data, sizeof(public_key_raw_data));
#endif
    }

    return public_key_raw_data;
}

void fido_cryptoauth_generate_sha256_hash(uint8_t *data, size_t data_size, uint8_t *hash_digest, size_t *hash_digest_size)
{
    // 初期化
    *hash_digest_size = 0;
    if (fido_cryptoauth_init()== false) {
        return;
    }

    ATCA_STATUS status = atcab_hw_sha2_256(data, data_size, hash_digest);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_generate_sha256_hash failed: atcab_hw_sha2_256 returns 0x%02x", 
            status);

    } else {
        *hash_digest_size = ATCA_SHA_DIGEST_SIZE;
    }
}

void fido_cryptoauth_generate_random_vector(uint8_t *vector_buf, size_t vector_buf_size)
{
    // 初期化
    memset(m_random_vector, 0x00, sizeof(m_random_vector));
    memset(vector_buf, 0x00, vector_buf_size);
    if (fido_cryptoauth_init()== false) {
        return;
    }

    // ランダムベクターを生成
    ATCA_STATUS status = atcab_random(m_random_vector);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_generate_random_vector failed (1): atcab_random returns 0x%02x", 
            status);
        return;
    }

    // 引数で指定のバイト数分、配列に格納（最大32バイトまで）
    memcpy(vector_buf, m_random_vector, 
        vector_buf_size < RANDOM_NUM_SIZE ? vector_buf_size : RANDOM_NUM_SIZE);
}

void fido_cryptoauth_ecdsa_sign(uint16_t key_id, uint8_t const *hash_digest, uint8_t *signature, size_t *signature_size)
{
    // 初期化
    *signature_size = 0;
    if (fido_cryptoauth_init()== false) {
        return;
    }

    // 署名実行
    ATCA_STATUS status = atcab_sign(key_id, hash_digest, signature);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_ecdsa_sign failed: atcab_sign(%d) returns 0x%02x", 
            key_id, status);

    } else {
        *signature_size = ATCA_SIG_SIZE;
#if LOG_HEXDUMP_DEBUG_SIGN
        fido_log_debug("fido_cryptoauth_ecdsa_sign (key_id=%d):", key_id);
        fido_log_print_hexdump_debug(signature, *signature_size);
#endif
#if ATCAB_VERIFY_EXTERN
        // 署名を公開鍵で検証
        bool is_verified = false;
        uint8_t *p_public_key = fido_cryptoauth_keypair_public_key(key_id);
        status = atcab_verify_extern(hash_digest, signature, p_public_key, &is_verified);
        if (status != ATCA_SUCCESS) {
            fido_log_error("fido_cryptoauth_ecdsa_sign failed: atcab_verify_extern(%d) returns 0x%02x", 
                key_id, status);
        } else {
            fido_log_debug("fido_cryptoauth_ecdsa_sign verified: %s", 
                is_verified ? "true" : "false");
        }
#endif
    }
}

//
// HMAC SHA-256ハッシュ関連処理
//
// HMAC SHA-256ハッシュ生成用キーの一時格納領域
static uint8_t hmac_sha256_key_tmp[32];

bool fido_cryptoauth_calculate_hmac_sha256(
    uint8_t *key_data, size_t key_data_size, 
    uint8_t *src_data, size_t src_data_size, uint8_t *src_data_2, size_t src_data_2_size,
    uint8_t *dest_data)
{
    // 初期化
    ATCA_STATUS status;
    memset(dest_data, 0x00, HMAC_DIGEST_SIZE);
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    // 引数のキーを、一時領域に32バイトまでコピー
    // 長さが32バイト未満の場合、末尾に 0x00 を埋める
    size_t max_key_length = sizeof(hmac_sha256_key_tmp);
    memset(hmac_sha256_key_tmp, 0x00, max_key_length);
    memcpy(hmac_sha256_key_tmp, key_data, 
        key_data_size < max_key_length ? key_data_size : max_key_length);

    // 8番スロットにキーを32バイトまで書き込み
    uint16_t key_id = KEY_ID_FOR_CONFIDENTIAL_DATA;
    status = atcab_write_zone(ATCA_ZONE_DATA, key_id, 
        BLOCK_IDX_FOR_HMAC_GENERATE_KEY, 0, hmac_sha256_key_tmp, max_key_length);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_calculate_hmac_sha256 failed: atcab_write_zone(%d) returns 0x%02x", 
            key_id, status);
        return false;
    }

#if LOG_HEXDUMP_HMAC256_KEY
    fido_log_debug("fido_cryptoauth_calculate_hmac_sha256 use key (key_id=%d):", key_id_for_hmac);
    status = atcab_read_zone(ATCA_ZONE_DATA, key_id_for_hmac, 0, 0, hmac_sha256_key_tmp, max_key_length);
    fido_log_print_hexdump_debug(hmac_sha256_key_tmp, max_key_length);
#endif

    // HMAC SHA-256ハッシュを計算
    atca_hmac_sha256_ctx_t ctx;
    status = atcab_sha_hmac_init(&ctx, key_id);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_calculate_hmac_sha256 failed: atcab_sha_hmac_init(%d) returns 0x%02x", 
            key_id, status);
        return false;
    }

    status = atcab_sha_hmac_update(&ctx, src_data, src_data_size);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_calculate_hmac_sha256 failed: atcab_sha_hmac_update(%d)[1] returns 0x%02x", 
            key_id, status);
        return false;
    }

    // 2番目の引数を計算対象に設定
    if (src_data_2 != NULL && src_data_2_size > 0) {
        status = atcab_sha_hmac_update(&ctx, src_data_2, src_data_2_size);
        if (status != ATCA_SUCCESS) {
            fido_log_error("fido_cryptoauth_calculate_hmac_sha256 failed: atcab_sha_hmac_update(%d)[2] returns 0x%02x", 
                key_id, status);
            return false;
        }
    }

    status = atcab_sha_hmac_finish(&ctx, dest_data, SHA_MODE_TARGET_TEMPKEY);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_calculate_hmac_sha256 failed: atcab_sha_hmac_finish(%d) returns 0x%02x", 
            key_id, status);
        return false;
    }

    return true;
}

//
// ECDH共通鍵関連処理
//
// 共通鍵格納領域
//   この領域に格納される共通鍵(Shared secret key)は、
//   ビッグエンディアン配列となる
static uint8_t sskey_raw_data[ECDH_KEY_SIZE];

// 共通鍵ハッシュ格納領域
static uint8_t sskey_hash[ATCA_SHA_DIGEST_SIZE];

// 鍵交換用キーペアが生成済みかどうかを保持
static bool keypair_generated = false;

void fido_cryptoauth_sskey_init(bool force)
{
    // 鍵交換用キーペアが生成済みで、かつ
    // 強制再生成を要求しない場合は終了
    if (keypair_generated && force == false) {
        NRF_LOG_DEBUG("Keypair for exchanging key is already exist");
        return;
    }

    // 秘密鍵および公開鍵を、所定のスロットで生成
    fido_cryptoauth_keypair_generate(KEY_ID_FOR_SHARED_SECRET_KEY);

    // 生成済みフラグを設定
    if (!keypair_generated) {
        NRF_LOG_DEBUG("Keypair for exchanging key generate success");
    } else {
        NRF_LOG_DEBUG("Keypair for exchanging key re-generate success");
    }
    keypair_generated = true;
}

bool fido_cryptoauth_sskey_generate(uint8_t *client_public_key_raw_data)
{
    // 初期化
    ATCA_STATUS status;
    memset(sskey_raw_data, 0x00, sizeof(sskey_raw_data));
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    // 鍵交換用キーペアが未生成の場合は終了
    if (!keypair_generated) {
        NRF_LOG_ERROR("Keypair for exchanging key is not exist");
        return false;
    }

    // 共通鍵を生成
    status = atcab_ecdh(KEY_ID_FOR_SHARED_SECRET_KEY, client_public_key_raw_data, sskey_raw_data);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_sskey_generate failed: atcab_ecdh(%d) returns 0x%02x", 
            KEY_ID_FOR_SHARED_SECRET_KEY, status);
        return false;

#if LOG_HEXDUMP_DEBUG_SSKEY
    } else {
        fido_log_debug("fido_cryptoauth_sskey_generate (key_id=%d):", KEY_ID_FOR_SHARED_SECRET_KEY);
        fido_log_print_hexdump_debug(sskey_raw_data, sizeof(sskey_raw_data));
#endif
    }

    // 生成した共通鍵をSHA-256ハッシュ化し、
    // 共通鍵ハッシュ（32バイト）を作成
    size_t sskey_hash_size;
    fido_cryptoauth_generate_sha256_hash(sskey_raw_data, sizeof(sskey_raw_data), sskey_hash, &sskey_hash_size);
    return true;
}

uint8_t *fido_cryptoauth_sskey_public_key(void)
{
    // ECDH共通鍵生成時の秘密鍵に対応する公開鍵を取得して戻す
    return fido_cryptoauth_keypair_public_key(KEY_ID_FOR_SHARED_SECRET_KEY);
}

uint8_t *fido_cryptoauth_sskey_hash(void)
{
    return sskey_hash;
}

//
// 外部秘密鍵導入関連処理
//
bool fido_cryptoauth_install_privkey(uint8_t *private_key_raw_data)
{
    // 初期化
    ATCA_STATUS status;
    if (fido_cryptoauth_init()== false) {
        return false;
    }

    // 32バイトの一時キーを生成
    status = atcab_random(m_random_vector);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_install_privkey failed: atcab_random returns 0x%02x", 
            status);
        return false;
    }

    // 一時キーを１５番スロットに書込み
    status = atcab_write_zone(ATCA_ZONE_DATA, KEY_ID_FOR_INSTALL_PRV_TMP_KEY, 0, 0, m_random_vector, ATCA_BLOCK_SIZE);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_install_privkey failed: atcab_write_zone(%d) returns 0x%02x", 
            KEY_ID_FOR_INSTALL_PRV_TMP_KEY, status);
        return false;
    }

    // 秘密鍵を一時バッファにセット（先頭の４バイトは 0 埋めとする）
    memset(private_key_buffer, 0x00, sizeof(private_key_buffer));
    memcpy(private_key_buffer + 4, private_key_raw_data, ATCA_PRIV_KEY_SIZE);

    // 秘密鍵を１４番スロットに書込み
    //   １５番スロットには、１４番スロットに書込まれた
    //   秘密鍵を暗号化するキーが上書き保存されます。
    //   １４・１５番スロットの内容は、
    //   いかなる手段によっても参照することができません。
    status = atcab_priv_write(KEY_ID_FOR_INSTALL_PRIVATE_KEY, private_key_buffer, KEY_ID_FOR_INSTALL_PRV_TMP_KEY, m_random_vector);
    if (status != ATCA_SUCCESS) {
        fido_log_error("fido_cryptoauth_install_privkey failed: atcab_priv_write(%d) returns 0x%02x", 
            KEY_ID_FOR_INSTALL_PRIVATE_KEY, status);
        return false;
    }

    return true;
}

bool fido_cryptoauth_extract_pubkey_from_cert(uint8_t *public_key, uint8_t *cert_data, size_t cert_data_length)
{
    // 開始バイトが不正な場合は終了
    if (cert_data[0] != 0x30) {
        fido_log_error("fido_cryptoauth_extract_pubkey_from_cert failed: Invalid certificate");
        return false;
    }

    for (size_t i = 3; i < cert_data_length; i++) {
        if (cert_data[i-3] == 0x03 && cert_data[i-2] == 0x42 &&
            cert_data[i-1] == 0x00 && cert_data[i]   == 0x04) {
            // 03 42 00 04 というシーケンスが発見されたら、
            // その後ろから64バイト分のデータをコピー
            memcpy(public_key, cert_data + i + 1, ATCA_PUB_KEY_SIZE);
            return true;
        }
    }
    
    fido_log_error("fido_cryptoauth_extract_pubkey_from_cert failed: Public key not found");
    return false;
}
