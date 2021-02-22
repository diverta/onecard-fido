/* 
 * File:   ccid_openpgp_key_rsa.c
 * Author: makmorit
 *
 * Created on 2021/02/18, 15:05
 */
#include "ccid_openpgp.h"
#include "ccid_openpgp_key.h"
#include "ccid_openpgp_key_rsa.h"
#include "ccid_openpgp_object.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// テスト用
#define LOG_DEBUG_PKEY_BUFFER       false

// 鍵格納領域
//   Offset
//     0: P (private key)
//   128: Q (private key)
//   256: N (public key)
static uint8_t m_rsa_key[512];

uint8_t *ccid_openpgp_key_rsa_private_key(void)
{
    return m_rsa_key;
}

uint8_t *ccid_openpgp_key_rsa_public_key(void)
{
    return m_rsa_key + 256;
}

static void rsa_process_timer_handler(void)
{
    // キープアライブ（Time extension）レスポンスを送信
    ccid_response_time_extension();
}

static void rsa_process_timer_start(void)
{
    // キープアライブタイマーを開始
    fido_repeat_process_timer_start(1500, rsa_process_timer_handler);
}

static uint16_t rsa_process_terminate(uint16_t sw)
{
    // キープアライブタイマーを停止
    fido_repeat_process_timer_stop();
    return sw;
}

#if LOG_DEBUG_PKEY_BUFFER
static void log_buffer(void)
{
    fido_log_debug("Private key first 32 bytes: ");
    fido_log_print_hexdump_debug(m_rsa_key, 32);
    fido_log_debug(" last 32 bytes: ");
    fido_log_print_hexdump_debug(m_rsa_key + 224, 32);

    fido_log_debug("Public key first 32 bytes: ");
    fido_log_print_hexdump_debug(m_rsa_key + 256, 32);
    fido_log_debug(" last 32 bytes: ");
    fido_log_print_hexdump_debug(m_rsa_key + 256 + 224, 32);
}
#endif

uint16_t ccid_openpgp_key_rsa_generate(uint8_t *key_attr)
{
    // ビット数を取得
    unsigned int nbits = (key_attr[1] << 8) | key_attr[2];
    if (nbits != 2048) {
        // RSA-2048以外はサポート外
        fido_log_error("OpenPGP do not support RSA-%d ", nbits);
        return SW_WRONG_DATA;
    }

    // キープアライブタイマーを開始
    rsa_process_timer_start();

    // RSA秘密鍵を生成
    if (ccid_crypto_rsa_generate_key(m_rsa_key, m_rsa_key + 256, nbits) == false) {
        return rsa_process_terminate(SW_UNABLE_TO_PROCESS);
    }

    fido_log_info("OpenPGP key pair (RSA-2048) generate done");

#if LOG_DEBUG_PKEY_BUFFER
    log_buffer();
#endif

    // 正常終了
    return rsa_process_terminate(SW_NO_ERROR);
}

uint16_t ccid_openpgp_key_rsa_read(uint16_t key_tag)
{
    // 領域を初期化
    memset(m_rsa_key, 0, sizeof(m_rsa_key));

    // Flash ROMから秘密鍵・公開鍵を取得
    uint8_t *buf;
    if (ccid_openpgp_object_data_get(key_tag, &buf, NULL) == false) {
        // 読出しが失敗した場合はエラー
        fido_log_error("OpenPGP private key read fail: tag=0x%04x", key_tag);
        return SW_UNABLE_TO_PROCESS;
    }

    // 領域にコピー
    memcpy(m_rsa_key, buf, sizeof(m_rsa_key));

#if LOG_DEBUG_PKEY_BUFFER
    log_buffer();
#endif

    // 正常終了
    return SW_NO_ERROR;
}
