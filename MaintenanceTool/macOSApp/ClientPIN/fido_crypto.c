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
#include <openssl/sha.h>

// pinHashEnc:
//   Encrypted first 16 bytes of SHA-256 hash of curPin
//   using sharedSecret
//   AES256-CBC(sharedSecret, IV= 0, LEFT(SHA-256(curPin),16))
static uint8_t pinHashEnc[16];

static uint8_t sha256(fido_blob_t *data, fido_blob_t *digest)
{
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

uint8_t generate_pin_hash_enc(char *old_pin) {
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
    fido_blob_set(pin, (uint8_t *)old_pin, strlen(old_pin));
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

