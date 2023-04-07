//
//  fido_crypto.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include <stdlib.h>
#include <string.h>

#include "aes_256_cbc.h"
#include "fido_crypto.h"
#include "fido_blob.h"
#include "debug_log.h"
#include "FIDODefines.h"
#include "tool_ecdh.h"

// for OpenSSL
#include <openssl/core_names.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

// for generate public key from private key
#include <openssl/ec.h>
#include <openssl/bn.h>

// pinHashEnc:
//   Encrypted first 16 bytes of SHA-256 hash of curPin
//   using sharedSecret
//   AES256-CBC(sharedSecret, IV= 0, LEFT(SHA-256(curPin),16))
static uint8_t pinHashEnc[16];

// newPinEnc: Encrypted newPin using sharedSecret
//   AES256-CBC(sharedSecret, IV=0, newPin)
static uint8_t newPinEnc[256];
static size_t  newPinEncSize;

// pinAuth
//   setPIN: LEFT(HMAC-SHA-256(sharedSecret, newPinEnc), 16)
//   changePIN: LEFT(HMAC-SHA-256(sharedSecret, newPinEnc || pinHashEnc), 16)
static uint8_t pinAuth[16];

// clientDataHash: Hash of the serialized client data
//   SHA-256(clientData)
static uint8_t clientDataHash[32];

// saltEnc: Encrypt two salts using sharedSecret
//   AES256-CBC(sharedSecret, IV=0, salt1 (32 bytes) || salt2 (32 bytes))
static uint8_t saltEnc[64];
static size_t  saltEncSize;

// saltAuth
//   LEFT(HMAC-SHA-256(sharedSecret, saltEnc), 16)
static uint8_t saltAuth[16];

uint8_t *pin_hash_enc(void) {
    return pinHashEnc;
}

uint8_t *new_pin_enc(void) {
    return newPinEnc;
}

size_t new_pin_enc_size(void) {
    return newPinEncSize;
}

uint8_t *pin_auth(void) {
    return pinAuth;
}

uint8_t *client_data_hash(void) {
    return clientDataHash;
}

uint8_t *salt_enc(void) {
    return saltEnc;
}

uint8_t *salt_auth(void) {
    return saltAuth;
}

static uint8_t sha256(fido_blob_t *data, fido_blob_t *digest) {
    if ((digest->ptr = calloc(1, SHA256_DIGEST_LENGTH)) == NULL) {
        return CTAP1_ERR_OTHER;
    }
    
    digest->len = SHA256_DIGEST_LENGTH;
    if (SHA256(data->ptr, data->len, digest->ptr) != digest->ptr) {
        free(digest->ptr);
        digest->ptr = NULL;
        digest->len = 0;
        return CTAP1_ERR_OTHER;
    }
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t generate_pin_hash_enc(const char *cur_pin) {
    fido_blob_t *pin = NULL;
    fido_blob_t *ph = NULL;
    fido_blob_t *shared = NULL;
    fido_blob_t *phe = NULL;
    uint8_t      ok = CTAP1_ERR_OTHER;

    // 作業領域の確保
    memset(pinHashEnc, 0, sizeof(pinHashEnc));
    if ((pin = fido_blob_new()) == NULL ||
        (ph = fido_blob_new()) == NULL ||
        (shared = fido_blob_new()) == NULL ||
        (phe = fido_blob_new()) == NULL) {
        goto fail;
    }
    // PINコードハッシュを生成（16バイト）
    fido_blob_set(pin, (uint8_t *)cur_pin, strlen(cur_pin));
    if (sha256(pin, ph) != CTAP1_ERR_SUCCESS || ph->len < 16) {
        log_debug("%s: SHA256", __func__);
        goto fail;
    }
    ph->len = 16;
    // 共通鍵を使用し暗号化
    fido_blob_set(shared, tool_ecdh_shared_secret_key(), 32);
    size_t size = sizeof(pinHashEnc);
    if (aes_256_cbc_enc(shared->ptr, ph->ptr, ph->len, pinHashEnc, &size)) {
        ok = CTAP1_ERR_SUCCESS;
    }
    
fail:
    // 作業領域を解放
    fido_blob_free(&pin);
    fido_blob_free(&ph);
    fido_blob_free(&shared);
    fido_blob_free(&phe);
    
    return ok;
}

static uint8_t pad64(const char *pin, fido_blob_t **ppin) {
    size_t pin_len;
    size_t ppin_len;
    
    pin_len = strlen(pin);
    if (pin_len < 4 || pin_len > 255) {
        log_debug("%s: invalid pin length", __func__);
        return CTAP2_ERR_PIN_POLICY_VIOLATION;
    }
    
    if ((*ppin = fido_blob_new()) == NULL)
        return CTAP1_ERR_OTHER;
    
    ppin_len = (pin_len + 63) & ~63;
    if (ppin_len < pin_len || ((*ppin)->ptr = calloc(1, ppin_len)) == NULL) {
        fido_blob_free(ppin);
        return CTAP1_ERR_OTHER;
    }
    
    memcpy((*ppin)->ptr, pin, pin_len);
    (*ppin)->len = ppin_len;
    
    return CTAP1_ERR_SUCCESS;
}

uint8_t generate_new_pin_enc(const char *new_pin) {
    fido_blob_t *ppin;
    fido_blob_t *key;
    fido_blob_t *pe;
    uint8_t      ok = CTAP1_ERR_OTHER;

    // PINコードを固定長化
    uint8_t ret = pad64(new_pin, &ppin);
    if (ret != CTAP1_ERR_SUCCESS) {
        return ret;
    }
    // 作業領域の確保
    memset(newPinEnc, 0, sizeof(newPinEnc));
    if ((key = fido_blob_new()) == NULL ||
        (pe = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 共通鍵を使用し、PINコードを暗号化
    fido_blob_set(key, tool_ecdh_shared_secret_key(), 32);
    newPinEncSize = sizeof(newPinEnc);
    if (aes_256_cbc_enc(key->ptr, ppin->ptr, ppin->len, newPinEnc, &newPinEncSize)) {
        ok = CTAP1_ERR_SUCCESS;
    }

fail:
    // 作業領域を解放
    fido_blob_free(&ppin);
    fido_blob_free(&key);
    fido_blob_free(&pe);

    return ok;
}

uint8_t generate_pin_auth(bool change_pin) {
    uint8_t       dgst[SHA256_DIGEST_LENGTH];
    size_t        dgst_len = SHA256_DIGEST_LENGTH;
    EVP_MAC      *mac = NULL;
    EVP_MAC_CTX  *ctx = NULL;
    OSSL_PARAM    params[2];
    uint8_t       ok = CTAP1_ERR_OTHER;

    // 作業領域の確保
    memset(pinAuth, 0, sizeof(pinAuth));
    // 共通鍵と暗号化されたPINコードを使用し、pinAuthを生成
    if ((mac = EVP_MAC_fetch(NULL, OSSL_MAC_NAME_HMAC, NULL)) == NULL) {
        log_debug("%s: EVP_MAC_fetch", __func__);
        goto fail;
    }
    if ((ctx = EVP_MAC_CTX_new(mac)) == NULL) {
        log_debug("%s: EVP_MAC_CTX_new", __func__);
        goto fail;
    }
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_ALG_PARAM_DIGEST, OSSL_DIGEST_NAME_SHA2_256, 0);
    params[1] = OSSL_PARAM_construct_end();
    if (EVP_MAC_init(ctx, tool_ecdh_shared_secret_key(), 32, params) == 0) {
        log_debug("%s: EVP_MAC_init", __func__);
        goto fail;
    }
    if (EVP_MAC_update(ctx, newPinEnc, newPinEncSize) == 0) {
        log_debug("%s: EVP_MAC_update(newPinEnc)", __func__);
        goto fail;
    }
    if (change_pin) {
        if (EVP_MAC_update(ctx, pinHashEnc, sizeof(pinHashEnc)) == 0) {
            log_debug("%s: EVP_MAC_update(pinHashEnc)", __func__);
            goto fail;
        }
    }
    if (EVP_MAC_final(ctx, dgst, &dgst_len, sizeof(dgst)) == 0 || dgst_len != SHA256_DIGEST_LENGTH) {
        log_debug("%s: EVP_MAC_final", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(pinAuth, dgst, sizeof(pinAuth));
    log_debug("%s success", __func__);
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    if (ctx != NULL) {
        EVP_MAC_CTX_free(ctx);
    }
    if (mac != NULL) {
        EVP_MAC_free(mac);
    }
    return ok;
}

uint8_t decrypto_pin_token(
    uint8_t *encrypted_pin_token, uint8_t *decrypted_pin_token, size_t pin_token_size) {
    fido_blob_t *key;
    fido_blob_t *pe;
    fido_blob_t *pd;
    uint8_t      ok = CTAP1_ERR_OTHER;
    
    // 作業領域の確保
    memset(decrypted_pin_token, 0, pin_token_size);
    if ((key = fido_blob_new()) == NULL ||
        (pe = fido_blob_new()) == NULL ||
        (pd = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 共通鍵を使用し、PINコードを復号化
    fido_blob_set(key, tool_ecdh_shared_secret_key(), 32);
    fido_blob_set(pe, encrypted_pin_token, pin_token_size);
    size_t size = pin_token_size;
    if (aes_256_cbc_dec(key->ptr, pe->ptr, pe->len, decrypted_pin_token, &size)) {
        ok = CTAP1_ERR_SUCCESS;
    }
    
fail:
    // 作業領域を解放
    fido_blob_free(&key);
    fido_blob_free(&pe);
    fido_blob_free(&pd);

    return ok;
}

uint8_t generate_client_data_hash(const char *client_data) {
    fido_blob_t *cd = NULL;
    fido_blob_t *cdh = NULL;
    uint8_t      ok = CTAP1_ERR_OTHER;
    
    // 作業領域の確保
    memset(clientDataHash, 0, sizeof(clientDataHash));
    if ((cd = fido_blob_new()) == NULL ||
        (cdh = fido_blob_new()) == NULL) {
        goto fail;
    }
    // ClientDataハッシュを生成（32バイト）
    fido_blob_set(cd, (uint8_t *)client_data, strlen(client_data));
    if (sha256(cd, cdh) != CTAP1_ERR_SUCCESS || cdh->len < 32) {
        log_debug("%s: SHA256", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(clientDataHash, cdh->ptr, cdh->len);
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    // 作業領域を解放
    fido_blob_free(&cd);
    fido_blob_free(&cdh);
    
    return ok;
}

uint8_t generate_pin_auth_from_client_data(uint8_t *decrypted_pin_token, uint8_t *client_data_hash) {
    uint8_t       dgst[SHA256_DIGEST_LENGTH];
    size_t        dgst_len = SHA256_DIGEST_LENGTH;
    EVP_MAC      *mac = NULL;
    EVP_MAC_CTX  *ctx = NULL;
    OSSL_PARAM    params[2];
    uint8_t       ok = CTAP1_ERR_OTHER;
    size_t        pin_token_size = 16;
    size_t        client_data_hash_size = SHA256_DIGEST_LENGTH;
    
    // 作業領域の確保
    memset(pinAuth, 0, sizeof(pinAuth));
    // 復号化されたPINトークンと、clientDataHashを使用し、pinAuthを生成
    if ((mac = EVP_MAC_fetch(NULL, OSSL_MAC_NAME_HMAC, NULL)) == NULL) {
        log_debug("%s: EVP_MAC_fetch", __func__);
        goto fail;
    }
    if ((ctx = EVP_MAC_CTX_new(mac)) == NULL) {
        log_debug("%s: EVP_MAC_CTX_new", __func__);
        goto fail;
    }
    params[0] = OSSL_PARAM_construct_utf8_string(OSSL_ALG_PARAM_DIGEST, OSSL_DIGEST_NAME_SHA2_256, 0);
    params[1] = OSSL_PARAM_construct_end();
    if (EVP_MAC_init(ctx, decrypted_pin_token, pin_token_size, params) == 0) {
        log_debug("%s: EVP_MAC_init", __func__);
        goto fail;
    }
    if (EVP_MAC_update(ctx, client_data_hash, client_data_hash_size) == 0) {
        log_debug("%s: EVP_MAC_update(clientDataHash)", __func__);
        goto fail;
    }
    if (EVP_MAC_final(ctx, dgst, &dgst_len, sizeof(dgst)) == 0 || dgst_len != SHA256_DIGEST_LENGTH) {
        log_debug("%s: EVP_MAC_final", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(pinAuth, dgst, sizeof(pinAuth));
    log_debug("%s success", __func__);
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    if (ctx != NULL) {
        EVP_MAC_CTX_free(ctx);
    }
    if (mac != NULL) {
        EVP_MAC_free(mac);
    }
    return ok;
}

uint8_t generate_salt_enc(uint8_t *hmac_secret_salt, size_t hmac_secret_salt_size) {
    fido_blob_t *psalt;
    fido_blob_t *key;
    fido_blob_t *pe;
    uint8_t      ok = CTAP1_ERR_OTHER;

    // 作業領域の確保
    memset(saltEnc, 0, sizeof(saltEnc));
    if ((psalt = fido_blob_new()) == NULL ||
        (key = fido_blob_new()) == NULL ||
        (pe = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 共通鍵を使用し、PINコードを暗号化
    fido_blob_set(psalt, hmac_secret_salt, hmac_secret_salt_size);
    fido_blob_set(key, tool_ecdh_shared_secret_key(), 32);
    saltEncSize = sizeof(saltEnc);
    if (aes_256_cbc_enc(key->ptr, psalt->ptr, psalt->len, saltEnc, &saltEncSize)) {
        ok = CTAP1_ERR_SUCCESS;
    }
    
fail:
    // 作業領域を解放
    fido_blob_free(&psalt);
    fido_blob_free(&key);
    fido_blob_free(&pe);
    
    return ok;
}

uint8_t generate_salt_auth(uint8_t *salt_enc, size_t salt_enc_size) {
    uint8_t       dgst[SHA256_DIGEST_LENGTH];
    unsigned int  dgst_len = SHA256_DIGEST_LENGTH;
    const EVP_MD *md = NULL;
    HMAC_CTX     *ctx = NULL;
    fido_blob_t  *key;
    uint8_t       ok = CTAP1_ERR_OTHER;
    
    // 作業領域の確保
    memset(saltAuth, 0, sizeof(saltAuth));
    if ((key = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 共通鍵と暗号化されたsaltを使用し、saltAuthを生成
    fido_blob_set(key, tool_ecdh_shared_secret_key(), 32);
    if ((ctx = HMAC_CTX_new()) == NULL) {
        log_debug("%s: HMAC_CTX_new", __func__);
        goto fail;
    }
    if ((md = EVP_sha256()) == NULL) {
        log_debug("%s: EVP_sha256", __func__);
        goto fail;
    }
    if (HMAC_Init_ex(ctx, key->ptr, (int)key->len, md, NULL) == 0) {
        log_debug("%s: HMAC_Init_ex", __func__);
        goto fail;
    }
    if (HMAC_Update(ctx, salt_enc, salt_enc_size) == 0) {
        log_debug("%s: HMAC_Update(saltEnc)", __func__);
        goto fail;
    }
    if (HMAC_Final(ctx, dgst, &dgst_len) == 0 || dgst_len != SHA256_DIGEST_LENGTH) {
        log_debug("%s: HMAC_Final", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(saltAuth, dgst, sizeof(saltAuth));
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    // 作業領域を解放
    fido_blob_free(&key);
    
    return ok;
}

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
    const EC_GROUP *group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
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
    if (EC_POINT_point2oct(group, ec_point, POINT_CONVERSION_UNCOMPRESSED, conv_buf, sizeof(conv_buf), ctx) == 0) {
        log_debug("%s: EC_POINT_point2oct failed", __func__);
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
