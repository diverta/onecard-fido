//
//  fido_crypto.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include <stdlib.h>
#include <string.h>

#include "fido_crypto.h"
#include "fido_blob.h"
#include "debug_log.h"
#include "FIDODefines.h"
#include "AES256CBC.h"
#include "ECDH.h"

// for OpenSSL
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

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
    fido_blob_set(shared, ECDH_shared_secret_key(), 32);
    if (aes256_cbc_enc(shared, ph, phe) < 0) {
        log_debug("%s: aes256_cbc_enc", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(pinHashEnc, phe->ptr, phe->len);
    ok = CTAP1_ERR_SUCCESS;
    
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
    fido_blob_set(key, ECDH_shared_secret_key(), 32);
    if (aes256_cbc_enc(key, ppin, pe) < 0) {
        goto fail;
    }

    // 配列に退避
    memcpy(newPinEnc, pe->ptr, pe->len);
    newPinEncSize = pe->len;
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    // 作業領域を解放
    fido_blob_free(&ppin);
    fido_blob_free(&key);
    fido_blob_free(&pe);

    return ok;
}

uint8_t generate_pin_auth(bool change_pin) {
    uint8_t       dgst[SHA256_DIGEST_LENGTH];
    unsigned int  dgst_len = SHA256_DIGEST_LENGTH;
    const EVP_MD *md = NULL;
    HMAC_CTX     *ctx = NULL;
    fido_blob_t  *key;
    uint8_t       ok = CTAP1_ERR_OTHER;

    // 作業領域の確保
    memset(pinAuth, 0, sizeof(pinAuth));
    if ((key = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 共通鍵と暗号化されたPINコードを使用し、pinAuthを生成
    fido_blob_set(key, ECDH_shared_secret_key(), 32);
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
    if (HMAC_Update(ctx, newPinEnc, newPinEncSize) == 0) {
        log_debug("%s: HMAC_Update(newPinEnc)", __func__);
        goto fail;
    }
    if (change_pin) {
        if (HMAC_Update(ctx, pinHashEnc, sizeof(pinHashEnc)) == 0) {
            log_debug("%s: HMAC_Update(pinHashEnc)", __func__);
            goto fail;
        }
    }
    if (HMAC_Final(ctx, dgst, &dgst_len) == 0 || dgst_len != SHA256_DIGEST_LENGTH) {
        log_debug("%s: HMAC_Final", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(pinAuth, dgst, sizeof(pinAuth));
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    // 作業領域を解放
    fido_blob_free(&key);
    
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
    fido_blob_set(key, ECDH_shared_secret_key(), 32);
    fido_blob_set(pe, encrypted_pin_token, pin_token_size);
    if (aes256_cbc_dec(key, pe, pd) < 0) {
        goto fail;
    }
    // 配列に退避
    memcpy(decrypted_pin_token, pd->ptr, pin_token_size);
    ok = CTAP1_ERR_SUCCESS;
    
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
    unsigned int  dgst_len = SHA256_DIGEST_LENGTH;
    const EVP_MD *md = NULL;
    HMAC_CTX     *ctx = NULL;
    fido_blob_t  *key;
    uint8_t       ok = CTAP1_ERR_OTHER;
    size_t        pin_token_size = 16;
    size_t        client_data_hash_size = SHA256_DIGEST_LENGTH;
    
    // 作業領域の確保
    memset(pinAuth, 0, sizeof(pinAuth));
    if ((key = fido_blob_new()) == NULL) {
        goto fail;
    }
    // 復号化されたPINトークンと、clientDataHashを使用し、pinAuthを生成
    fido_blob_set(key, decrypted_pin_token, pin_token_size);
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
    if (HMAC_Update(ctx, client_data_hash, client_data_hash_size) == 0) {
        log_debug("%s: HMAC_Update(clientDataHash)", __func__);
        goto fail;
    }
    if (HMAC_Final(ctx, dgst, &dgst_len) == 0 || dgst_len != SHA256_DIGEST_LENGTH) {
        log_debug("%s: HMAC_Final", __func__);
        goto fail;
    }
    // 配列に退避
    memcpy(pinAuth, dgst, sizeof(pinAuth));
    ok = CTAP1_ERR_SUCCESS;
    
fail:
    // 作業領域を解放
    fido_blob_free(&key);
    
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
    fido_blob_set(key, ECDH_shared_secret_key(), 32);
    if (aes256_cbc_enc(key, psalt, pe) < 0) {
        goto fail;
    }
    
    // 配列に退避
    memcpy(saltEnc, pe->ptr, pe->len);
    saltEncSize = pe->len;
    ok = CTAP1_ERR_SUCCESS;
    
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
    fido_blob_set(key, ECDH_shared_secret_key(), 32);
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
