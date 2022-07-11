//
//  fido_crypto.c
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/22.
//
#include <stdlib.h>
#include <string.h>

#include "fido_crypto.h"
#include "FIDODefines.h"

// for OpenSSL
#include <openssl/evp.h>

// for generate public key from private key
#include <openssl/ec.h>
#include <openssl/bn.h>

// メッセージを保持
static char debug_message[1024];

char *log_debug_message(void) {
    return debug_message;
}

static void log_debug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsprintf(debug_message, fmt, ap);
    va_end(ap);
}

//
// 鍵・証明書インストール関連処理
//
// 公開鍵の妥当性検証用
static uint8_t pubkey_from_cert[64];
static uint8_t pubkey_from_privkey[64];

static bool extract_pubkey_from_cert(uint8_t *public_key, uint8_t *cert_data, size_t cert_data_length)
{
    // 開始バイトが不正な場合は終了
    if (cert_data[0] != 0x30) {
        return false;
    }
    
    for (size_t i = 3; i < cert_data_length; i++) {
        if (cert_data[i-3] == 0x03 && cert_data[i-2] == 0x42 &&
            cert_data[i-1] == 0x00 && cert_data[i]   == 0x04) {
            // 03 42 00 04 というシーケンスが発見されたら、
            // その後ろから64バイト分のデータをコピー
            memcpy(public_key, cert_data + i + 1, 64);
            return true;
        }
    }
    
    return false;
}

static bool generate_pubkey_from_privkey(uint8_t *public_key, uint8_t *skey_bytes, size_t skey_bytes_size) {
    uint8_t conv_buf[65];
    bool ret = false;

    // 作業領域の確保
    BIGNUM *bn_public_key = BN_new();
    BIGNUM *bn_private_key = BN_new();
    BN_CTX *ctx = BN_CTX_new();
    if (bn_public_key == NULL || bn_private_key == NULL || ctx == NULL) {
        log_debug("%s: BN_new failed", __func__);
        goto fail;
    }

    // 秘密鍵を内部形式に変換
    if (BN_bin2bn(skey_bytes, (int)skey_bytes_size, bn_private_key) == 0) {
        log_debug("%s: BN_bin2bn failed", __func__);
        goto fail;
    }

    // EC_POINTを生成
    EC_KEY *eckey = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    const EC_GROUP *group = EC_KEY_get0_group(eckey);
    EC_POINT *ec_point = EC_POINT_new(group);
    if (ec_point == NULL) {
        log_debug("%s: EC_POINT_new failed", __func__);
        goto fail;
    }

    // 秘密鍵から公開鍵を生成
    if (EC_POINT_mul(group, ec_point, bn_private_key, NULL, NULL, ctx) == 0) {
        log_debug("%s: EC_POINT_mul failed", __func__);
        goto fail;
    }

    // 内部形式の公開鍵を、バイトデータに変換
    if (EC_POINT_point2bn(group, ec_point, POINT_CONVERSION_UNCOMPRESSED, bn_public_key, ctx) == NULL) {
        log_debug("%s: EC_POINT_point2bn failed", __func__);
        goto fail;
    }
    if (BN_bn2bin(bn_public_key, conv_buf) == 0) {
        log_debug("%s: BN_bn2bin failed", __func__);
        goto fail;
    }

    // 先頭の１バイトを除いたデータを、引数の領域にコピー
    log_debug("%s: success", __func__);
    memcpy(public_key, conv_buf + 1, 64);
    ret = true;

fail:
    // 作業領域を解放
    BN_CTX_free(ctx);
    BN_free(bn_public_key);
    BN_free(bn_private_key);
    return ret;
}

uint8_t validate_skey_cert(uint8_t *skey_bytes, size_t skey_bytes_size,
                           uint8_t *cert_bytes, size_t cert_bytes_size) {
    // 証明書から公開鍵を抽出
    if (extract_pubkey_from_cert(pubkey_from_cert, cert_bytes, cert_bytes_size) == false) {
        return CTAP1_ERR_OTHER;
    }

    // 秘密鍵から公開鍵を生成
    if (generate_pubkey_from_privkey(pubkey_from_privkey, skey_bytes, skey_bytes_size) == false) {
        return CTAP1_ERR_OTHER;
    }

    // 両者を比較
    if (memcmp(pubkey_from_cert, pubkey_from_privkey, 64) == 0) {
        return CTAP1_ERR_SUCCESS;

    } else {
        log_debug("%s: Public key (from certificate) does not match public key (from private key)", __func__);
        return CTAP1_ERR_OTHER;
    }
}
