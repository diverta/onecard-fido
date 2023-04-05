//
//  ECDH.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include <stdbool.h>
#include "debug_log.h"

// for OpenSSL
#include <openssl/core_names.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/param_build.h>
#include <openssl/sha.h>

// 生成された鍵を保持
static uint8_t      pubkey_work[65];
static uint8_t      seckey_work[SHA256_DIGEST_LENGTH];
static uint8_t      shared_secret_key[SHA256_DIGEST_LENGTH];
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
    OSSL_PARAM_BLD *param_bld = NULL;
    OSSL_PARAM     *params    = NULL;
    EVP_PKEY_CTX   *ctx       = NULL;
    EVP_PKEY       *pkey      = NULL;
    
    param_bld = OSSL_PARAM_BLD_new();
    if (param_bld == NULL) {
        log_debug("%s: OSSL_PARAM_BLD_new", __func__);
        goto fail;
    }
    if (OSSL_PARAM_BLD_push_utf8_string(param_bld, OSSL_PKEY_PARAM_GROUP_NAME, SN_X9_62_prime256v1, 0) == 0) {
        log_debug("%s: OSSL_PARAM_BLD_push_utf8_string", __func__);
        goto fail;
    }
    
    // Construct parameters
    pubkey_work[0] = POINT_CONVERSION_UNCOMPRESSED;
    memcpy(pubkey_work + 1,  agreement_pubkey_X, 32);
    memcpy(pubkey_work + 33, agreement_pubkey_Y, 32);
    if (OSSL_PARAM_BLD_push_octet_string(param_bld, OSSL_PKEY_PARAM_PUB_KEY, pubkey_work, sizeof(pubkey_work)) == 0) {
        log_debug("%s: OSSL_PARAM_BLD_push_octet_string", __func__);
        goto fail;
    }
    params = OSSL_PARAM_BLD_to_param(param_bld);
    if (params == NULL) {
        log_debug("%s: OSSL_PARAM_BLD_to_param", __func__);
        goto fail;
    }
    ctx = EVP_PKEY_CTX_new_from_name(NULL, "EC", NULL);
    if (ctx == NULL) {
        log_debug("%s: EVP_PKEY_CTX_new_from_name", __func__);
        goto fail;
    }
    
    // Convert agreement pubkey to EVP_PKEY
    int ret_code;
    if ((ret_code = EVP_PKEY_fromdata_init(ctx)) <= 0) {
        log_debug("%s: EVP_PKEY_fromdata_init returns %d", __func__, ret_code);
        goto fail;
    }
    if ((ret_code = EVP_PKEY_fromdata(ctx, &pkey, EVP_PKEY_PUBLIC_KEY, params)) <= 0) {
        log_debug("%s: EVP_PKEY_fromdata returns %d", __func__, ret_code);
        goto fail;
    }

fail:
    if (param_bld != NULL)
        OSSL_PARAM_BLD_free(param_bld);
    if (params != NULL)
        OSSL_PARAM_free(params);
    if (ctx != NULL)
        EVP_PKEY_CTX_free(ctx);
    
    return pkey;
}

static bool perform_ecdh(EVP_PKEY *sk_evp, EVP_PKEY *pk_evp) {
    bool         ret  = false;
    EVP_PKEY_CTX *ctx = NULL;
    
    /* set ecdh parameters */
    if ((ctx = EVP_PKEY_CTX_new(sk_evp, NULL)) == NULL ||
        EVP_PKEY_derive_init(ctx) <= 0 ||
        EVP_PKEY_derive_set_peer(ctx, pk_evp) <= 0) {
        log_debug("%s: EVP_PKEY_derive_init", __func__);
        goto fail;
    }
    
    /* perform ecdh */
    size_t size = sizeof(seckey_work);
    if (EVP_PKEY_derive(ctx, seckey_work, &size) <= 0) {
        log_debug("%s: EVP_PKEY_derive", __func__);
        goto fail;
    }
    
    /* use sha256 as a kdf on the resulting secret */
    if (SHA256(seckey_work, sizeof(seckey_work), shared_secret_key) == NULL) {
        log_debug("%s: SHA256", __func__);
        goto fail;
    }
    ret = true;

fail:
    if (ctx != NULL)
        EVP_PKEY_CTX_free(ctx);
    
    return ret;
}

bool ECDH_create_shared_secret_key(uint8_t *agreement_pubkey_X, uint8_t *agreement_pubkey_Y) {
    bool        ret   = false;
    EVP_PKEY    *pkey = NULL;
    EVP_PKEY    *qkey = NULL;
    // ECDHで使用するキーペアを新規生成
    if ((pkey = generate_keypair_for_ecdh()) == NULL) {
        goto fail;
    }
    // 受領した公開鍵を内部形式に変換
    if ((qkey = convert_pubkey_for_ecdh(agreement_pubkey_X, agreement_pubkey_Y)) == NULL) {
        goto fail;
    }
    // 共通鍵を生成
    if (perform_ecdh(pkey, qkey) == false) {
        goto fail;
    }
    ret = true;

fail:
    if (pkey != NULL)
        EVP_PKEY_free(pkey);
    if (qkey != NULL)
        EVP_PKEY_free(qkey);

    return ret;
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
