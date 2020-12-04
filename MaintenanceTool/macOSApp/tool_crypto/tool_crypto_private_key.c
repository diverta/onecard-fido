//
//  tool_crypto_private_key.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#include <string.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include "debug_log.h"
#include "tool_crypto_common.h"
#include "tool_crypto_private_key.h"

// RSA-2048秘密鍵要素を格納
static unsigned char e[4];
static unsigned char p[RSA2048_PQ_SIZE];
static unsigned char q[RSA2048_PQ_SIZE];
static unsigned char dmp1[RSA2048_PQ_SIZE];
static unsigned char dmq1[RSA2048_PQ_SIZE];
static unsigned char iqmp[RSA2048_PQ_SIZE];

// ECCP-256秘密鍵要素を格納
static unsigned char m_ec_pk[ECCP256_KEY_SIZE];

//
// references for memory
//
static EVP_PKEY    *m_private_key = NULL;
static RSA         *m_rsa_private_key = NULL;
static EC_KEY      *m_ec_private_key = NULL;
static FILE        *m_input_file = NULL;

static void initialize_references(void)
{
    m_private_key = NULL;
    m_rsa_private_key = NULL;
    m_input_file = NULL;
}

//
// terminate functions
//
static bool extract_rsa_2048_terminate(bool success)
{
    if (m_rsa_private_key != NULL) {
        RSA_free(m_rsa_private_key);
    }
    return success;
}

static bool extract_eccp_256_terminate(bool success)
{
    if (m_ec_private_key != NULL) {
        EC_KEY_free(m_ec_private_key);
    }
    return success;
}

static bool extract_from_pem_terminate(bool success)
{
    if (m_input_file != NULL) {
        fclose(m_input_file);
    }
    if (m_private_key != NULL) {
        EVP_PKEY_free(m_private_key);
    }
    return success;
}

//
// private functions
//
static unsigned char get_algorithm(EVP_PKEY *key)
{
    int type = EVP_PKEY_base_id(key);
    int size = EVP_PKEY_bits(key);

    if (type == EVP_PKEY_RSA && size == RSA2048_N_SIZE) {
        return CRYPTO_ALG_RSA2048;
    }
    if (type == EVP_PKEY_EC && size == 256) {
        return CRYPTO_ALG_ECCP256;
    }
    log_debug("%s: Unsupported algorithm (type=0x%04x, size=%d)", __func__, type, size);
    return CRYPTO_ALG_NONE;
}

static bool set_component(unsigned char *in_ptr, const BIGNUM *bn, int element_len)
{
    int real_len = BN_num_bytes(bn);
    if (real_len > element_len) {
        return false;
    }
    memset(in_ptr, 0, (size_t)(element_len - real_len));
    in_ptr += element_len - real_len;
    BN_bn2bin(bn, in_ptr);
    return true;
}

static bool is_valid_exponent(unsigned char *e)
{
    return (e[0] == 0x01 && e[1] == 0x00 && e[2] == 0x01);
}

static bool extract_rsa_2048(EVP_PKEY *private_key, unsigned char *pkey_data, size_t *pkey_size)
{
    const BIGNUM *bn_e, *bn_p, *bn_q, *bn_dmp1, *bn_dmq1, *bn_iqmp;
    m_rsa_private_key = EVP_PKEY_get1_RSA(private_key);
    if (m_rsa_private_key == NULL) {
        log_debug("%s: Invalid RSA private key", __func__);
        return extract_rsa_2048_terminate(false);
    }
    RSA_get0_key(m_rsa_private_key, NULL, &bn_e, NULL);
    RSA_get0_factors(m_rsa_private_key, &bn_p, &bn_q);
    RSA_get0_crt_params(m_rsa_private_key, &bn_dmp1, &bn_dmq1, &bn_iqmp);

    // 秘密鍵の各要素を抽出
    if ((set_component(e, bn_e, 3) == false) || is_valid_exponent(e) == false) {
        log_debug("%s: Invalid public exponent for import (only 0x10001 supported)", __func__);
        return extract_rsa_2048_terminate(false);
    }
    if (set_component(p, bn_p, RSA2048_PQ_SIZE) == false) {
        log_debug("%s: Failed setting P component", __func__);
        return extract_rsa_2048_terminate(false);
    }
    if (set_component(q, bn_q, RSA2048_PQ_SIZE) == false) {
        log_debug("%s: Failed setting Q component", __func__);
        return extract_rsa_2048_terminate(false);
    }
    if (set_component(dmp1, bn_dmp1, RSA2048_PQ_SIZE) == false) {
        log_debug("%s: Failed setting DP component", __func__);
        return extract_rsa_2048_terminate(false);
    }
    if (set_component(dmq1, bn_dmq1, RSA2048_PQ_SIZE) == false) {
        log_debug("%s: Failed setting DQ component", __func__);
        return extract_rsa_2048_terminate(false);
    }
    if (set_component(iqmp, bn_iqmp, RSA2048_PQ_SIZE) == false) {
        log_debug("%s: Failed setting QINV component", __func__);
        return extract_rsa_2048_terminate(false);
    }
    // 引数の領域にコピー
    size_t offset = 0;
    memcpy(pkey_data + offset, p, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    memcpy(pkey_data + offset, q, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    memcpy(pkey_data + offset, dmp1, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    memcpy(pkey_data + offset, dmq1, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    memcpy(pkey_data + offset, iqmp, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    *pkey_size = offset;
    return extract_rsa_2048_terminate(true);
}

static bool extract_eccp_256(EVP_PKEY *private_key, unsigned char *pkey_data, size_t *pkey_size)
{
    m_ec_private_key = EVP_PKEY_get1_EC_KEY(private_key);
    if (m_ec_private_key == NULL) {
        log_debug("%s: Invalid EC private key", __func__);
        return extract_eccp_256_terminate(false);
    }
    const BIGNUM *s = EC_KEY_get0_private_key(m_ec_private_key);

    // 秘密鍵の要素を抽出
    if (set_component(m_ec_pk, s, ECCP256_KEY_SIZE) == false) {
        log_debug("%s: Failed setting EC private key", __func__);
        return extract_eccp_256_terminate(false);
    }
    // 引数の領域にコピー
    memcpy(pkey_data, m_ec_pk, ECCP256_KEY_SIZE);
    *pkey_size = ECCP256_KEY_SIZE;
    return extract_eccp_256_terminate(true);
}

//
// public functions
//
bool tool_crypto_private_key_extract_from_pem(const char *pem_path, unsigned char *alg, unsigned char *pkey_data, size_t *pkey_size)
{
    // 変数を初期化
    initialize_references();
    char *password = NULL;

    // PEMファイルを開く
    m_input_file = fopen(pem_path, "r");
    if (m_input_file == NULL) {
        log_debug("%s: Pem file open fail (%s)", __func__, pem_path);
        return extract_from_pem_terminate(false);
    }
    // PEMファイルから秘密鍵を読込
    m_private_key = PEM_read_PrivateKey(m_input_file, NULL, NULL, password);
    if (m_private_key == NULL) {
        log_debug("%s: Failed loading private key for import (%s)", __func__, pem_path);
        return extract_from_pem_terminate(false);
    }
    // アルゴリズムに応じて処理分岐
    bool ret = false;
    *alg = get_algorithm(m_private_key);
    switch (*alg) {
        case CRYPTO_ALG_RSA2048:
            // RSA-2048秘密鍵を抽出
            ret = extract_rsa_2048(m_private_key, pkey_data, pkey_size);
            break;
        case CRYPTO_ALG_ECCP256:
            // ECCP-256秘密鍵を抽出
            ret = extract_eccp_256(m_private_key, pkey_data, pkey_size);
            break;
        default:
            break;
    }
    // 処理終了
    return extract_from_pem_terminate(ret);
}
