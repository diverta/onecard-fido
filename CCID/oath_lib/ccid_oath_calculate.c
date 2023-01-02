/* 
 * File:   ccid_oath_calculate.c
 * Author: makmorit
 *
 * Created on 2022/07/11, 17:03
 */
#include "ccid_define.h"
#include "ccid_oath.h"
#include "ccid_oath_calculate.h"
#include "ccid_oath_define.h"
#include "ccid_oath_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// for debug
#define LOG_ACCOUNT_EXIST_AND_READ      false
#define LOG_CALCULATED_DIGEST           false

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_oath_calculate);
#endif

//
// アカウントデータ格納用
//
static char    m_account_name[MAX_NAME_LEN];
static uint8_t m_secret[MAX_KEY_LEN];
static uint8_t m_property;
static uint8_t m_challenge[MAX_CHALLENGE_LEN];
//
// OTP計算用
//
static uint8_t m_challenge_temp[MAX_CHALLENGE_LEN];
static uint8_t m_digest[SHA256_DIGEST_LENGTH];

uint16_t ccid_oath_calculate(void *p_capdu, void *p_rapdu) 
{
    // パラメーターのチェック
    command_apdu_t  *capdu = (command_apdu_t *)p_capdu;
    response_apdu_t *rapdu = (response_apdu_t *)p_rapdu;
    if (capdu->p1 != 0x00 || capdu->p2 != 0x00) {
        return SW_WRONG_P1P2;
    }

    //
    // アカウント名を抽出
    //
    uint8_t offset = 0;
    if (offset + 1 >= capdu->lc) {
        return(SW_WRONG_LENGTH);
    }
    if (capdu->data[offset++] != OATH_TAG_NAME) {
        return SW_WRONG_DATA;
    }
    uint8_t name_len = capdu->data[offset++];
    if (name_len > MAX_NAME_LEN || name_len == 0) {
        return SW_WRONG_DATA;
    }
    uint8_t name_offset = offset;
    offset += name_len;
    if (offset > capdu->lc) {
        return SW_WRONG_LENGTH;
    }

    // アカウント名をバッファにコピー
    memset(m_account_name, 0, sizeof(m_account_name));
    memcpy(m_account_name, capdu->data + name_offset, name_len);

    //
    // アカウントデータを取得
    //
    bool exist = false;
    uint8_t secret_size;
    uint16_t sw = ccid_oath_object_account_read(m_account_name, name_len, (char *)m_secret, &secret_size, &m_property, m_challenge, &exist);

#if LOG_ACCOUNT_EXIST_AND_READ
    fido_log_debug("account record(%s): exist=%d", log_strdup(m_account_name), exist);
    fido_log_print_hexdump_debug(m_secret, secret_size, "secret");
    fido_log_print_hexdump_debug(m_challenge, MAX_CHALLENGE_LEN, "challenge");
#endif
    if (exist == false) {
        return SW_DATA_INVALID;
    }

    uint8_t challenge_size = 0;
    if ((m_secret[0] & OATH_TYPE_MASK) == OATH_TYPE_TOTP) {
        // 受信Challengeをチェック
        if (offset + 1 >= capdu->lc) {
            return SW_WRONG_LENGTH;
        }
        if (capdu->data[offset++] != OATH_TAG_CHALLENGE) {
            return SW_WRONG_DATA;
        }
        challenge_size = capdu->data[offset++];
        if (challenge_size > MAX_CHALLENGE_LEN || challenge_size == 0) {
            return SW_WRONG_DATA;
        }
        if (offset + challenge_size > capdu->lc) {
            return SW_WRONG_LENGTH;
        }

        // 受信Challengeを一時領域に格納
        memset(m_challenge_temp, 0, sizeof(m_challenge_temp));
        memcpy(m_challenge_temp, capdu->data + offset, challenge_size);
        offset += challenge_size;
        if (offset > capdu->lc) {
            return SW_WRONG_LENGTH;
        }

    } else if ((m_secret[0] & OATH_TYPE_MASK) == OATH_TYPE_HOTP) {
        // TODO: 後日実装予定
    
    } else {
        return SW_DATA_INVALID;
    }

    // レスポンスを生成
    rapdu->data[0] = OATH_TAG_RESPONSE;
    rapdu->data[1] = 5;
    rapdu->data[2] = m_secret[1];

    // 計算されたOTPをセット（４バイト）
    uint8_t *p_otp = rapdu->data + 3;
    ccid_oath_calculate_digest(m_secret, secret_size, m_challenge_temp, challenge_size, p_otp);

    // レスポンス長＝７バイト
    rapdu->len = 7;
    return sw;
}

static void calculate_hmac_sha1(uint8_t *secret, uint8_t secret_size, uint8_t *challenge, uint8_t challenge_size, uint8_t *digest)
{
#if LOG_CALCULATED_DIGEST
    fido_log_print_hexdump_debug(secret, secret_size, "Secret for HMAC SHA-1");
    fido_log_print_hexdump_debug(challenge, challenge_size, "Challenge for HMAC SHA-1");
#endif

    fido_crypto_calculate_hmac_sha1(secret, secret_size, challenge, challenge_size, NULL, 0, digest);
}

static void calculate_hmac_sha256(uint8_t *secret, uint8_t secret_size, uint8_t *challenge, uint8_t challenge_size, uint8_t *digest)
{
#if LOG_CALCULATED_DIGEST
    fido_log_print_hexdump_debug(secret, secret_size, "Secret for HMAC SHA-256");
    fido_log_print_hexdump_debug(challenge, challenge_size, "Challenge for HMAC SHA-256");
#endif

    fido_crypto_calculate_hmac_sha256(secret, secret_size, challenge, challenge_size, NULL, 0, digest);
}

void ccid_oath_calculate_digest(uint8_t *secret, uint8_t secret_size, uint8_t *challenge, uint8_t challenge_size, uint8_t *buffer) 
{
    // ハッシュ値を計算
    uint8_t digest_length;
    if ((secret[0] & OATH_ALG_MASK) == OATH_ALG_SHA1) {
        // SHA-1ハッシュを計算
        calculate_hmac_sha1(m_secret + 2, secret_size - 2, challenge, challenge_size, m_digest);
        digest_length = SHA1_DIGEST_LENGTH;

    } else {
        // SHA-256ハッシュを計算
        calculate_hmac_sha256(m_secret + 2, secret_size - 2, challenge, challenge_size, m_digest);
        digest_length = SHA256_DIGEST_LENGTH;
    }

    // ハッシュ値配列の末尾バイトから、４ビット分の数値を抽出
    // （0 - 15までの数値）--> offsetに設定
    uint8_t offset = m_digest[digest_length - 1] & 0xF;

    // offsetから後方31ビット分を得るため、
    // offsetの先頭バイトからは7ビットのみ抽出しておく
    m_digest[offset] &= 0x7F;

    // 引数の領域に、offsetの先頭から４バイト分格納
    memcpy(buffer, m_digest + offset, 4);

#if LOG_CALCULATED_DIGEST
    fido_log_print_hexdump_debug(m_digest, digest_length, "Calculated HMAC value");
    fido_log_print_hexdump_debug(buffer, 4, "Calculated OTP digest value");
#endif
}
