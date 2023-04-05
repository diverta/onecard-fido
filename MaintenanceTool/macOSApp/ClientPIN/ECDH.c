//
//  ECDH.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include "ECDH.h"
#include "FIDODefines.h"
#include "debug_log.h"

// for OpenSSL
#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

// 生成された鍵を保持
static uint8_t      pubkey_work[65];
static uint8_t      shared_secret_key[32];
static uint8_t      public_key_X[32];
static uint8_t      public_key_Y[32];

static EVP_PKEY *generate_keypair_for_ecdh(void) {
    EVP_PKEY_CTX    *pctx = NULL;
    EVP_PKEY_CTX    *kctx = NULL;
    EVP_PKEY        *p = NULL;
    EVP_PKEY        *k = NULL;
    const int       nid = NID_X9_62_prime256v1;
    
    if ((pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL)) == NULL ||
        EVP_PKEY_paramgen_init(pctx) <= 0 ||
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, nid) <= 0 ||
        EVP_PKEY_paramgen(pctx, &p) <= 0) {
        log_debug("%s: EVP_PKEY_paramgen", __func__);
        goto fail;
    }
    
    if ((kctx = EVP_PKEY_CTX_new(p, NULL)) == NULL ||
        EVP_PKEY_keygen_init(kctx) <= 0 || EVP_PKEY_keygen(kctx, &k) <= 0) {
        log_debug("%s: EVP_PKEY_keygen", __func__);
        goto fail;
    }
    
    size_t size = 0;
    if (EVP_PKEY_get_octet_string_param(k, OSSL_PKEY_PARAM_PUB_KEY, pubkey_work, sizeof(pubkey_work), &size) == 0) {
        log_debug("%s: EVP_PKEY_get_octet_string_param", __func__);
        goto fail;
    }
    if (size != sizeof(pubkey_work)) {
        log_debug("%s: EVP_PKEY_get_octet_string_param: size=%d", __func__, size);
        goto fail;
    }
    // 生成された鍵を内部配列に保持
    memcpy(public_key_X, pubkey_work + 1,  32);
    memcpy(public_key_Y, pubkey_work + 33, 32);

fail:
    if (p != NULL)
        EVP_PKEY_free(p);
    if (pctx != NULL)
        EVP_PKEY_CTX_free(pctx);
    if (kctx != NULL)
        EVP_PKEY_CTX_free(kctx);
    
    return k;
}

static EVP_PKEY *convert_pubkey_for_ecdh(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y) {
    BN_CTX   *bnctx = NULL;
    EC_KEY   *ec = NULL;
    EC_POINT *q = NULL;
    EVP_PKEY *pkey = NULL;
    BIGNUM   *x = NULL;
    BIGNUM   *y = NULL;
    const EC_GROUP *g = NULL;
    const int nid = NID_X9_62_prime256v1;
    int       ok = -1;
    
    if ((bnctx = BN_CTX_new()) == NULL ||
        (x = BN_CTX_get(bnctx)) == NULL ||
        (y = BN_CTX_get(bnctx)) == NULL)
        goto fail;
    
    if (BN_bin2bn(agreement_pubkey_X, 32, x) == NULL ||
        BN_bin2bn(agreement_pubkey_Y, 32, y) == NULL) {
        log_debug("%s: BN_bin2bn", __func__);
        goto fail;
    }
    
    if ((ec = EC_KEY_new_by_curve_name(nid)) == NULL ||
        (g = EC_KEY_get0_group(ec)) == NULL) {
        log_debug("%s: EC_KEY init", __func__);
        goto fail;
    }
    
    if ((q = EC_POINT_new(g)) == NULL ||
        EC_POINT_set_affine_coordinates_GFp(g, q, x, y, bnctx) == 0 ||
        EC_KEY_set_public_key(ec, q) == 0) {
        log_debug("%s: EC_KEY_set_public_key", __func__);
        goto fail;
    }
    
    if ((pkey = EVP_PKEY_new()) == NULL ||
        EVP_PKEY_assign_EC_KEY(pkey, ec) == 0) {
        log_debug("%s: EVP_PKEY_assign_EC_KEY", __func__);
        goto fail;
    }
    
    ec = NULL; /* at this point, ec belongs to evp */
    ok = 0;

fail:
    if (bnctx != NULL)
        BN_CTX_free(bnctx);
    if (ec != NULL)
        EC_KEY_free(ec);
    if (q != NULL)
        EC_POINT_free(q);
    if (ok < 0 && pkey != NULL) {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }
    
    return pkey;
}

static uint8_t perform_ecdh(EVP_PKEY *sk_evp, EVP_PKEY *pk_evp, fido_blob_t **ecdh) {
    EVP_PKEY_CTX *ctx = NULL;
    fido_blob_t  *secret = NULL;
    uint8_t       ok = CTAP1_ERR_OTHER;
    
    *ecdh = NULL;   /* shared ecdh secret; returned */
    
    /* allocate blobs for secret & ecdh */
    if ((secret = fido_blob_new()) == NULL ||
        (*ecdh = fido_blob_new()) == NULL)
        goto fail;
    
    /* set ecdh parameters */
    if ((ctx = EVP_PKEY_CTX_new(sk_evp, NULL)) == NULL ||
        EVP_PKEY_derive_init(ctx) <= 0 ||
        EVP_PKEY_derive_set_peer(ctx, pk_evp) <= 0) {
        log_debug("%s: EVP_PKEY_derive_init", __func__);
        goto fail;
    }
    
    /* perform ecdh */
    if (EVP_PKEY_derive(ctx, NULL, &secret->len) <= 0 ||
        (secret->ptr = calloc(1, secret->len)) == NULL ||
        EVP_PKEY_derive(ctx, secret->ptr, &secret->len) <= 0) {
        log_debug("%s: EVP_PKEY_derive", __func__);
        goto fail;
    }
    
    /* use sha256 as a kdf on the resulting secret */
    (*ecdh)->len = SHA256_DIGEST_LENGTH;
    if (((*ecdh)->ptr = calloc(1, (*ecdh)->len)) == NULL ||
        SHA256(secret->ptr, secret->len, (*ecdh)->ptr) == NULL) {
        log_debug("%s: sha256", __func__);
        goto fail;
    }
    
    ok = CTAP1_ERR_SUCCESS;

fail:
    if (ctx != NULL)
        EVP_PKEY_CTX_free(ctx);
    if (ok < 0)
        fido_blob_free(ecdh);
    
    fido_blob_free(&secret);
    
    return ok;
}

uint8_t ECDH_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y) {
    uint8_t r;
    fido_blob_t *ecdh = NULL; /* shared ecdh secret; returned */
    EVP_PKEY    *pkey = NULL;
    EVP_PKEY    *qkey = NULL;

    // ECDHで使用するキーペアを新規生成
    if ((pkey = generate_keypair_for_ecdh()) == NULL) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    // 受領した公開鍵を内部形式に変換
    if ((qkey = convert_pubkey_for_ecdh(agreement_pubkey_X, agreement_pubkey_Y)) == NULL) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    // 共通鍵を生成
    if (perform_ecdh(pkey, qkey, &ecdh) < 0) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    r = CTAP1_ERR_SUCCESS;

fail:
    // 生成された鍵を内部配列に保持
    if (r == CTAP1_ERR_SUCCESS) {
        memcpy(shared_secret_key, ecdh->ptr, ecdh->len);
    }
    // 確保領域を解放
    if (ecdh != NULL)
        fido_blob_free(&ecdh);
    if (pkey != NULL)
        EVP_PKEY_free(pkey);
    if (qkey != NULL)
        EVP_PKEY_free(qkey);

    return r;
}

uint8_t *ECDH_shared_secret_key(void) {
    return shared_secret_key;
}

uint8_t *ECDH_public_key_X(void) {
    return public_key_X;
}

uint8_t *ECDH_public_key_Y(void) {
    return public_key_Y;
}
