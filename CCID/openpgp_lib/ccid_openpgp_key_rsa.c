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

uint16_t ccid_openpgp_key_rsa_nbits(uint8_t *key_attr, unsigned int *p_nbits)
{
    // ビット数を取得
    unsigned int nbits = (key_attr[1] << 8) | key_attr[2];
    if (nbits != 2048) {
        // RSA-2048以外はサポート外
        fido_log_error("OpenPGP do not support RSA-%d ", nbits);
        return SW_WRONG_DATA;
    }
    if (p_nbits != NULL) {
        *p_nbits = nbits;
    }
    return SW_NO_ERROR;
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

uint16_t ccid_openpgp_key_rsa_import(uint8_t *key_attr, uint8_t *privkey_pq)
{
    // ビット数を取得
    unsigned int nbits;
    uint16_t sw = ccid_openpgp_key_rsa_nbits(key_attr, &nbits);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // 秘密鍵を引数の領域からコピー
    size_t pq_size = nbits / 8;
    memcpy(ccid_openpgp_key_rsa_private_key(), privkey_pq, pq_size);

    // キープアライブタイマーを開始
    rsa_process_timer_start();

    // RSA秘密鍵をインポートし、同時に秘密鍵から公開鍵を生成
    if (ccid_crypto_rsa_import(ccid_openpgp_key_rsa_private_key(), ccid_openpgp_key_rsa_public_key(), nbits) == false) {
        return rsa_process_terminate(SW_UNABLE_TO_PROCESS);
    }

#if LOG_DEBUG_PKEY_BUFFER
    uint8_t *p = ccid_openpgp_key_rsa_private_key();
    fido_log_debug("pkey data first 256 bytes: ");
    fido_log_print_hexdump_debug(p, 64);
    fido_log_print_hexdump_debug(p + 64, 64);
    fido_log_print_hexdump_debug(p + 128, 64);
    fido_log_print_hexdump_debug(p + 192, 64);
#endif

    // 正常終了
    fido_log_info("OpenPGP public key (RSA-2048) import done");
    return rsa_process_terminate(SW_NO_ERROR);
}

uint16_t ccid_openpgp_key_rsa_generate(uint8_t *key_attr)
{
    // ビット数を取得
    unsigned int nbits;
    uint16_t sw = ccid_openpgp_key_rsa_nbits(key_attr, &nbits);
    if (sw != SW_NO_ERROR) {
        return sw;
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

    // Flash ROMに保管されている公開鍵がブランクの場合はエラー
    uint8_t *p = ccid_openpgp_key_rsa_public_key();
    if (p[0] == 0) {
        fido_log_error("OpenPGP public key is invalid: tag=0x%04x", key_tag);
        return SW_UNABLE_TO_PROCESS;
    }

    // 正常終了
    return SW_NO_ERROR;
}

static uint16_t pkcs1_v15_add_padding(const void *in, uint16_t in_size, uint8_t *out, uint16_t out_size) 
{
    // データ長チェック
    if (out_size < 11 || in_size > out_size - 11) {
        return SW_WRONG_DATA;
    }
    // データの余白サイズを計算
    uint16_t pad_size = out_size - in_size - 3;
    out[0] = 0x00;
    out[1] = 0x01;
    // データの左側余白に 0xff を埋める
    memset(out + 2, 0xff, pad_size);
    out[2 + pad_size] = 0x00;
    // データを右詰めでセット
    memcpy(out + pad_size + 3, in, in_size);
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_key_rsa_signature(uint16_t key_tag, uint8_t *key_attr, uint8_t *data, size_t size, uint8_t *signature, size_t *p_signature_size)
{
    // 鍵ビット数を取得
    unsigned int nbits;
    uint16_t sw = ccid_openpgp_key_rsa_nbits(key_attr, &nbits);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // Flash ROMから鍵を取得
    sw = ccid_openpgp_key_rsa_read(key_tag);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // データの余白埋め
    size_t signature_size = nbits / 8;
    sw = pkcs1_v15_add_padding(data, size, signature, signature_size);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // キープアライブタイマーを開始
    rsa_process_timer_start();

    // 署名生成
    if (ccid_crypto_rsa_private(ccid_openpgp_key_rsa_private_key(), signature, signature) == false) {
        return rsa_process_terminate(SW_UNABLE_TO_PROCESS);
    }

    // 正常終了
    if (p_signature_size != NULL) {
        *p_signature_size = signature_size;
    }
    fido_log_info("OpenPGP signature (RSA-2048) generate done");
    return rsa_process_terminate(SW_NO_ERROR);
}

static uint16_t pkcs1_v15_remove_padding(uint8_t *in, uint16_t in_size, uint8_t *out, size_t *out_size) 
{
    // データ長／内容をチェック
    if (in_size < 11) {
        return SW_WRONG_DATA;
    }
    if (in[0] != 0x00 || in[1] != 0x02) {
        return SW_WRONG_DATA;
    }
    // 余白直前の 0x00 バイトの位置を探す
    uint16_t i;
    for (i = 2; i < in_size; ++i) {
        if (in[i] == 0x00) {
            break;
        }
    }
    if (i == in_size || i - 2 < 8) {
        return SW_WRONG_DATA;
    }
    // データの余白サイズを計算
    uint16_t pad_size = i + 1;
    // データバイトを、余白サイズ分、左側へ移動
    memmove(out, in + pad_size, in_size - pad_size);
    // 余白を外したデータ長を戻す
    *out_size = in_size - pad_size;
    return SW_NO_ERROR;
}

uint16_t ccid_openpgp_key_rsa_decrypt(uint16_t key_tag, uint8_t *key_attr, uint8_t *encrypted, uint8_t *decrypted, size_t *p_decrypted_size)
{
    // 鍵ビット数を取得
    unsigned int nbits;
    uint16_t sw = ccid_openpgp_key_rsa_nbits(key_attr, &nbits);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // Flash ROMから鍵を取得
    sw = ccid_openpgp_key_rsa_read(key_tag);
    if (sw != SW_NO_ERROR) {
        return sw;
    }

    // キープアライブタイマーを開始
    rsa_process_timer_start();

    // 復号化
    if (ccid_crypto_rsa_private(ccid_openpgp_key_rsa_private_key(), encrypted, decrypted) == false) {
        return rsa_process_terminate(SW_UNABLE_TO_PROCESS);
    }

    // データの余白外し
    size_t size = nbits / 8;
    size_t decrypted_size;
    sw = pkcs1_v15_remove_padding(decrypted, size, decrypted, &decrypted_size);
    if (sw != SW_NO_ERROR) {
        return rsa_process_terminate(sw);
    }

    // 正常終了
    if (p_decrypted_size != NULL) {
        *p_decrypted_size = decrypted_size;
    }
    fido_log_info("OpenPGP decrypt (RSA-2048) done");
    return rsa_process_terminate(SW_NO_ERROR);
}
