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
static uint8_t      created_pubkey[65];
static uint8_t      shared_secret_key[32];
static uint8_t      public_key_X[32];
static uint8_t      public_key_Y[32];

static es256_sk_t *es256_sk_new(void) {
    return (es256_sk_t *)calloc(1UL, sizeof(es256_sk_t));
}

static void es256_sk_free(es256_sk_t **skp) {
    es256_sk_t *sk;
    
    if (skp == NULL || (sk = *skp) == NULL)
        return;
    
    explicit_bzero(sk, sizeof(es256_sk_t));
    free(sk);
    
    *skp = NULL;
}

static es256_pk_t *es256_pk_new(void) {
    return (es256_pk_t *)calloc(1UL, sizeof(es256_pk_t));
}

static void es256_pk_free(es256_pk_t **pkp) {
    es256_pk_t *pk;
    
    if (pkp == NULL || (pk = *pkp) == NULL)
        return;
    
    explicit_bzero(pk, sizeof(es256_pk_t));
    free(pk);
    
    *pkp = NULL;
}

static void es256_pk_set_x(es256_pk_t *pk, const unsigned char *x) {
    memcpy(pk->x, x, sizeof(pk->x));
}

static void es256_pk_set_y(es256_pk_t *pk, const unsigned char *y) {
    memcpy(pk->y, y, sizeof(pk->y));
}

static uint8_t es256_sk_create(es256_sk_t *key, es256_pk_t *pubkey) {
    EVP_PKEY_CTX    *pctx = NULL;
    EVP_PKEY_CTX    *kctx = NULL;
    EVP_PKEY        *p = NULL;
    EVP_PKEY        *k = NULL;
    BIGNUM          *d = NULL;
    const int       nid = NID_X9_62_prime256v1;
    int             n;
    uint8_t         ok = CTAP1_ERR_OTHER;
    
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
    
    if (EVP_PKEY_get_bn_param(k, OSSL_PKEY_PARAM_PRIV_KEY, &d) == 0 ||
        (n = BN_num_bytes(d)) < 0 || (size_t)n > sizeof(key->d) ||
        (n = BN_bn2bin(d, key->d)) < 0 || (size_t)n > sizeof(key->d)) {
        log_debug("%s: EVP_PKEY_get_bn_param", __func__);
        goto fail;
    }
    
    size_t size = 0;
    if (EVP_PKEY_get_octet_string_param(k, OSSL_PKEY_PARAM_PUB_KEY, created_pubkey, sizeof(created_pubkey), &size) == 0) {
        log_debug("%s: EVP_PKEY_get_octet_string_param", __func__);
        goto fail;
    }
    if (size != sizeof(created_pubkey)) {
        log_debug("%s: EVP_PKEY_get_octet_string_param: size=%d", __func__, size);
        goto fail;
    }
    memcpy(pubkey->x, created_pubkey + 1,  32);
    memcpy(pubkey->y, created_pubkey + 33, 32);
    ok = CTAP1_ERR_SUCCESS;

fail:
    if (p != NULL)
        EVP_PKEY_free(p);
    if (k != NULL)
        EVP_PKEY_free(k);
    if (pctx != NULL)
        EVP_PKEY_CTX_free(pctx);
    if (kctx != NULL)
        EVP_PKEY_CTX_free(kctx);
    if (d != NULL)
        BN_free(d);
    
    return ok;
}

EVP_PKEY *es256_pk_to_EVP_PKEY(const es256_pk_t *k) {
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
    
    if (BN_bin2bn(k->x, sizeof(k->x), x) == NULL ||
        BN_bin2bn(k->y, sizeof(k->y), y) == NULL) {
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

EVP_PKEY *es256_sk_to_EVP_PKEY(const es256_sk_t *k) {
    BN_CTX   *bnctx = NULL;
    EC_KEY   *ec = NULL;
    EVP_PKEY *pkey = NULL;
    BIGNUM   *d = NULL;
    const int nid = NID_X9_62_prime256v1;
    int       ok = -1;
    
    if ((bnctx = BN_CTX_new()) == NULL || (d = BN_CTX_get(bnctx)) == NULL ||
        BN_bin2bn(k->d, sizeof(k->d), d) == NULL) {
        log_debug("%s: BN_bin2bn", __func__);
        goto fail;
    }
    
    if ((ec = EC_KEY_new_by_curve_name(nid)) == NULL ||
        EC_KEY_set_private_key(ec, d) == 0) {
        log_debug("%s: EC_KEY_set_private_key", __func__);
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
    if (ok < 0 && pkey != NULL) {
        EVP_PKEY_free(pkey);
        pkey = NULL;
    }
    
    return pkey;
}

static uint8_t perform_ecdh(const es256_sk_t *sk, const es256_pk_t *pk, fido_blob_t **ecdh) {
    EVP_PKEY     *pk_evp = NULL;
    EVP_PKEY     *sk_evp = NULL;
    EVP_PKEY_CTX *ctx = NULL;
    fido_blob_t  *secret = NULL;
    uint8_t       ok = CTAP1_ERR_OTHER;
    
    *ecdh = NULL;   /* shared ecdh secret; returned */
    
    /* allocate blobs for secret & ecdh */
    if ((secret = fido_blob_new()) == NULL ||
        (*ecdh = fido_blob_new()) == NULL)
        goto fail;
    
    /* wrap the keys as openssl objects */
    if ((pk_evp = es256_pk_to_EVP_PKEY(pk)) == NULL ||
        (sk_evp = es256_sk_to_EVP_PKEY(sk)) == NULL) {
        log_debug("%s: es256_to_EVP_PKEY", __func__);
        goto fail;
    }
    
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
    if (pk_evp != NULL)
        EVP_PKEY_free(pk_evp);
    if (sk_evp != NULL)
        EVP_PKEY_free(sk_evp);
    if (ctx != NULL)
        EVP_PKEY_CTX_free(ctx);
    if (ok < 0)
        fido_blob_free(ecdh);
    
    fido_blob_free(&secret);
    
    return ok;
}

uint8_t ECDH_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y) {
    uint8_t r;
    es256_pk_t  *pk   = NULL; /* our public key; returned */
    es256_sk_t  *sk   = NULL; /* our private key */
    es256_pk_t  *ak   = NULL; /* authenticator's public key */
    fido_blob_t *ecdh = NULL; /* shared ecdh secret; returned */

    // 作業領域の確保
    if ((sk = es256_sk_new()) == NULL ||
        (pk = es256_pk_new()) == NULL ||
        (ak = es256_pk_new()) == NULL) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    // ECDHキーペアを新規生成
    if (es256_sk_create(sk, pk) != CTAP1_ERR_SUCCESS) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    // 受領した公開鍵を保持
    es256_pk_set_x(ak, agreement_pubkey_X);
    es256_pk_set_y(ak, agreement_pubkey_Y);
    
    // 共通鍵を生成
    if (perform_ecdh(sk, ak, &ecdh) < 0) {
        r = CTAP1_ERR_OTHER;
        goto fail;
    }
    
    r = CTAP1_ERR_SUCCESS;

fail:
    // 生成された鍵を内部配列に保持
    if (r == CTAP1_ERR_SUCCESS) {
        memcpy(shared_secret_key, ecdh->ptr, ecdh->len);
        memcpy(public_key_X, pk->x, 32);
        memcpy(public_key_Y, pk->y, 32);
    }
    // 確保領域を解放
    es256_sk_free(&sk);
    es256_pk_free(&ak);
    es256_pk_free(&pk);
    if (ecdh != NULL)
        fido_blob_free(&ecdh);

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
