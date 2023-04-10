//
//  AES256CBC.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include "AES256CBC.h"
#include "FIDODefines.h"
#include "debug_log.h"

// for OpenSSL
#include <openssl/evp.h>

uint8_t aes256_cbc_enc(const fido_blob_t *key, const fido_blob_t *in, fido_blob_t *out) {
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char   iv[32];
    int             len;
    uint8_t         ok = CTAP1_ERR_OTHER;
    
    memset(iv, 0, sizeof(iv));
    out->ptr = NULL;
    out->len = 0;
    
    /* sanity check */
    if (in->len > INT_MAX || (in->len % 16) != 0 ||
        (out->ptr = calloc(1, in->len)) == NULL) {
        log_debug("%s: in->len=%zu", __func__, in->len);
        goto fail;
    }
    
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL || key->len != 32 ||
        !EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key->ptr, iv) ||
        !EVP_CIPHER_CTX_set_padding(ctx, 0) ||
        !EVP_EncryptUpdate(ctx, out->ptr, &len, in->ptr, (int)in->len) ||
        len < 0 || (size_t)len != in->len) {
        log_debug("%s: EVP_Encrypt", __func__);
        goto fail;
    }
    
    out->len = (size_t)len;
    ok = CTAP1_ERR_SUCCESS;

fail:
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);
    
    if (ok != CTAP1_ERR_SUCCESS) {
        free(out->ptr);
        out->ptr = NULL;
        out->len = 0;
    }
    
    return ok;
}

uint8_t aes256_cbc_dec(const fido_blob_t *key, const fido_blob_t *in, fido_blob_t *out) {
    EVP_CIPHER_CTX *ctx = NULL;
    unsigned char   iv[32];
    int             len;
    uint8_t         ok = CTAP1_ERR_OTHER;
    
    memset(iv, 0, sizeof(iv));
    out->ptr = NULL;
    out->len = 0;
    
    /* sanity check */
    if (in->len > INT_MAX || (in->len % 16) != 0 ||
        (out->ptr = calloc(1, in->len)) == NULL) {
        log_debug("%s: in->len=%zu", __func__, in->len);
        goto fail;
    }
    
    if ((ctx = EVP_CIPHER_CTX_new()) == NULL || key->len != 32 ||
        !EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key->ptr, iv) ||
        !EVP_CIPHER_CTX_set_padding(ctx, 0) ||
        !EVP_DecryptUpdate(ctx, out->ptr, &len, in->ptr, (int)in->len) ||
        len < 0 || (size_t)len > in->len + 32) {
        log_debug("%s: EVP_Decrypt", __func__);
        goto fail;
    }
    
    out->len = (size_t)len;
    ok = CTAP1_ERR_SUCCESS;

fail:
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);
    
    if (ok != CTAP1_ERR_SUCCESS) {
        free(out->ptr);
        out->ptr = NULL;
        out->len = 0;
    }
    
    return ok;
}
