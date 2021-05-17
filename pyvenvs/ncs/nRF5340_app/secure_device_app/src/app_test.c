/* 
 * File:   app_test.c
 * Author: makmorit
 *
 * Created on 2021/05/17, 10:13
 */
#ifdef APP_TEST

#include <zephyr/types.h>
#include <zephyr.h>

// ログ出力制御
#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_test);

#define LOG_DEBUG_HID_REPORT        false
#define LOG_DEBUG_CCID_DATA         false

//
// for USB HID/CCID I/F test
//
#include "app_usb_hid.h"
#include "app_usb_ccid.h"

// 送受信データ格納用
static uint8_t m_request[64];
static uint8_t m_response[64];

static void test_ctap2hid_init(uint8_t *data, size_t size)
{
    // 受信フレームのデータを一時領域にコピー
    memcpy(m_request, data, size);

    // CTAPHID_INIT に対してレスポンスを実行
    size_t offset = 0;
    if (m_request[0] == 0xff && m_request[4] == 0x86) {
        offset = 6;
        memset(m_response, 0, sizeof(m_response));
        memcpy(m_response, m_request, offset);

        // bytes
        m_response[offset++] = 17;

        // nonce
        memcpy(m_response + offset, m_request + 7, 8);
        offset += 8;

        // CID 01 00 33 01
        m_response[offset+0] = 1;
        m_response[offset+1] = 0;
        m_response[offset+2] = 0x33;
        m_response[offset+3] = 1;
        offset += 4;

        // others
        m_response[offset++] = 2;
        m_response[offset++] = 5;
        m_response[offset++] = 0;
        m_response[offset++] = 2;
        m_response[offset++] = 1;

        // send report
        app_usb_hid_send_report(m_response, sizeof(m_response));
        LOG_DBG("sent %d bytes", sizeof(m_response));
        LOG_HEXDUMP_DBG(m_response, sizeof(m_response), "HID report");
    }
}

static uint8_t atr_ccid[] = {
    0x3b, 0xfd, 0x11, 0x00, 0x00, 0x81, 0x31, 0xfe, 0x65, 
    0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20, 0x44, 0x6f, 0x6e, 0x67, 0x6c, 0x65,
    0xfb
};
static uint8_t aid_ccid[] = {
    0x61, 0x11, 0x4F, 0x06, 0x00, 0x00, 0x10, 0x00, 
    0x01, 0x00, 0x79, 0x07, 0x4F, 0x05, 0xA0, 0x00, 
    0x00, 0x03, 0x08
};

static void test_ccid_status(uint8_t *data, size_t size)
{
    // 受信フレームのデータを一時領域にコピー
    memcpy(m_request, data, size);
    uint8_t seq = m_request[6];

    // スロットステータス取得に対してレスポンスを実行
    size_t offset = 0;
    memset(m_response, 0, sizeof(m_response));
    if (m_request[0] == 0x65 || m_request[0] == 0x63) {
        // command, seq
        m_response[0] = 0x81;
        m_response[6] = seq;
        m_response[8] = 0x81;
        offset = 10;
        app_usb_ccid_send_data(m_response, offset);

    } else if (m_request[0] == 0x62) {
        LOG_DBG("received %d bytes", size);
        LOG_HEXDUMP_DBG(data, size, "CCID data");

        m_response[0] = 0x80;
        m_response[1] = sizeof(atr_ccid);
        m_response[6] = seq;
        m_response[8] = 0x81;
        memcpy(m_response + 10, atr_ccid, sizeof(atr_ccid));
        offset = 10 + sizeof(atr_ccid);

        // send report
        if (app_usb_ccid_send_data(m_response, offset)) {
            LOG_DBG("sent %d bytes", offset);
            LOG_HEXDUMP_DBG(m_response, offset, "CCID data");
        }

    } else if (m_request[0] == 0x6f) {
        LOG_DBG("received %d bytes", size);
        LOG_HEXDUMP_DBG(data, size, "CCID data");

        m_response[0] = 0x80;
        m_response[1] = sizeof(aid_ccid) + 2;
        m_response[6] = seq;
        m_response[8] = 0x81;
        offset = 10;

        // aid
        memcpy(m_response + offset, aid_ccid, sizeof(aid_ccid));
        offset += sizeof(aid_ccid);

        // sw
        m_response[offset++] = 0x90;
        m_response[offset++] = 0x00;

        // send report
        if (app_usb_ccid_send_data(m_response, offset)) {
            LOG_DBG("sent %d bytes", offset);
            LOG_HEXDUMP_DBG(m_response, offset, "CCID data");
        }
    } 
}

//
// for crypto test
//
#include "app_crypto.h"
#include "app_crypto_define.h"

static uint8_t plaintext[64];
static uint8_t encrypted[64];
static uint8_t decrypted[64];
static uint8_t rand_buf[32];
static uint8_t work_buf[32];
static uint8_t hmac_sec_buf[32];

static char *plaintext_hex = "82fec664466d585023821c2e39a0c43345669a41244d05018a23d7159515f8ff4d88b01cd0eb83070d0077e065d74d7373816b61505718f8d4f270286a59d45e";
static char *hmac_sec = "141e6c2f7bfbec07a81e92b83bbe8da4e16f19a33ab2cb9b5aefe232b3ccc0bc";
static char *des3_key = "010203040506070801020304050607080102030405060708";

static void test_app_crypto(void)
{
    // テストデータ
    hex2bin(plaintext_hex, strlen(plaintext_hex), plaintext, sizeof(plaintext));
    LOG_HEXDUMP_DBG(plaintext, sizeof(plaintext), "Plaintext data for test");

    // AES暗号を生成
    if (app_crypto_generate_random_vector(rand_buf, sizeof(rand_buf)) == false) {
        return;
    }
    LOG_DBG("app_crypto_generate_random_vector done (%d bytes)", sizeof(rand_buf));
    LOG_HEXDUMP_DBG(rand_buf, sizeof(rand_buf), "AES key data");
 
    // テストデータを暗号化／復号化
    size_t size;
    if (app_crypto_aes_cbc_256_encrypt(rand_buf, plaintext, sizeof(plaintext), encrypted, &size) == false) {
        return;
    }
    LOG_DBG("app_crypto_aes_cbc_256_encrypt done");
    LOG_HEXDUMP_DBG(encrypted, size, "Encrypted data");

    if (app_crypto_aes_cbc_256_decrypt(rand_buf, encrypted, sizeof(encrypted), decrypted, &size) == false) {
        return;
    }
    LOG_DBG("app_crypto_aes_cbc_256_decrypt done");
    LOG_HEXDUMP_DBG(decrypted, size, "Decrypted data");

    // テストデータのハッシュを生成
    if (app_crypto_generate_sha256_hash(plaintext, sizeof(plaintext), work_buf) == false) {
        return;
    }
    LOG_DBG("app_crypto_generate_sha256_hash done");
    LOG_HEXDUMP_DBG(work_buf, SHA256_HASH_SIZE, "SHA-256 hash data");

    // HMACキーを生成
    hex2bin(hmac_sec, strlen(hmac_sec), hmac_sec_buf, sizeof(hmac_sec_buf));

    // テストデータのHMACハッシュを生成
    if (app_crypto_generate_hmac_sha256(hmac_sec_buf, sizeof(hmac_sec_buf), plaintext, 32, NULL, 0, work_buf) == false) {
        return;
    }
    LOG_DBG("app_crypto_generate_hmac_sha256 done (32 bytes)");
    LOG_HEXDUMP_DBG(work_buf, SHA256_HASH_SIZE, "HMAC SHA-256 hash data");

    if (app_crypto_generate_hmac_sha256(hmac_sec_buf, sizeof(hmac_sec_buf), plaintext, 32, plaintext+32, 32, work_buf) == false) {
        return;
    }
    LOG_DBG("app_crypto_generate_hmac_sha256 done (64 bytes)");
    LOG_HEXDUMP_DBG(work_buf, SHA256_HASH_SIZE, "HMAC SHA-256 hash data");

    // テストデータを3DESで暗号化
    hex2bin(des3_key, strlen(des3_key), work_buf, DES3_KEY_SIZE);
    if (app_crypto_des3_ecb(plaintext, encrypted, work_buf) == false) {
        return;
    }
    LOG_DBG("app_crypto_des3_ecb done");
    LOG_HEXDUMP_DBG(plaintext, DES3_CRYPTO_SIZE, "Plaintext data");
    LOG_HEXDUMP_DBG(encrypted, DES3_CRYPTO_SIZE, "3DES-ECB crypto data");

    // テスト正常完了
    LOG_DBG("Tests completed");
}

//
// エントリー関数
//
void app_custom_hid_report_received(uint8_t *data, size_t size)
{
#if LOG_DEBUG_HID_REPORT
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "HID report");
#endif

    test_ctap2hid_init(data, size);
}

void app_custom_hid_report_sent(void)
{
}

void app_custom_ccid_data_received(uint8_t *data, size_t size)
{
#if LOG_DEBUG_CCID_DATA
    LOG_DBG("received %d bytes", size);
    LOG_HEXDUMP_DBG(data, size, "CCID data");
#endif

    test_ccid_status(data, size);
}

void app_custom_button_pressed_short(void)
{
    test_app_crypto();
}

#endif /* APP_TEST */
