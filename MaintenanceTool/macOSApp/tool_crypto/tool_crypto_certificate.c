//
//  tool_crypto_certificate.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/02.
//
#include <string.h>
#include <openssl/pem.h>

#include "debug_log.h"
#include "tool_crypto_common.h"
#include "tool_crypto_certificate.h"
#include "ToolPivCommon.h"

// Cert desc info
static CERT_DESC   m_cert_desc;

//
// references for memory
//
static FILE        *m_input_file = NULL;
static X509        *m_certificate = NULL;
static EVP_PKEY    *m_public_key = NULL;

static void initialize_references(void)
{
    m_certificate = NULL;
    m_input_file = NULL;
    m_public_key = NULL;
}

//
// terminate functions
//
static bool extract_from_pem_terminate(bool success)
{
    if (m_certificate != NULL) {
        X509_free(m_certificate);
    }
    if (m_input_file != NULL) {
        fclose(m_input_file);
    }
    return success;
}

static bool extract_descriptions_terminate(bool success)
{
    if (m_certificate != NULL) {
        X509_free(m_certificate);
    }
    if (m_public_key != NULL) {
        EVP_PKEY_free(m_public_key);
    }
    return success;
}

//
// public functions
//
bool tool_crypto_certificate_extract_from_pem(const char *pem_path, unsigned char *cert_data, size_t *cert_size)
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
    m_certificate = PEM_read_X509(m_input_file, NULL, NULL, password);
    if (m_certificate == NULL) {
        log_debug("%s: Failed loading certificate for import (%s)", __func__, pem_path);
        return extract_from_pem_terminate(false);
    }
    // 証明書バイナリーイメージ長をチェック
    int cert_len = i2d_X509(m_certificate, NULL);
    if (cert_len > *cert_size || cert_len < 0) {
        log_debug("%s: Length of certificate (%d bytes) is more than can fit", __func__, cert_len);
        return extract_from_pem_terminate(false);
    }
    // 証明書バイナリーイメージを配列に抽出
    unsigned char *p_cert_data = cert_data;
    int cert_loaded = i2d_X509(m_certificate, &p_cert_data);
    if (cert_loaded != cert_len) {
        log_debug("%s: Failed loading certificate (size=%d, loaded=%d)", __func__, cert_len, cert_loaded);
        return extract_from_pem_terminate(false);
    }
    // 処理終了
    *cert_size = (size_t)cert_loaded;
    return extract_from_pem_terminate(true);
}

bool tool_crypto_certificate_extract_descriptions(unsigned char *cert_data, size_t cert_size)
{
    // 変数を初期化
    initialize_references();
    memset(&m_cert_desc, 0, sizeof(CERT_DESC));

    // 証明書を抽出
    const unsigned char *ptr = cert_data;
    unsigned long        cert_len = cert_size;
    m_certificate = d2i_X509(NULL, &ptr, cert_len);
    if (m_certificate == NULL) {
        log_debug("%s: Failed loading certificate for extract desc", __func__);
        return extract_descriptions_terminate(false);
    }
    // 公開鍵を抽出
    m_public_key = X509_get_pubkey(m_certificate);
    if (m_public_key == NULL) {
        log_debug("%s: Failed parsing public key for extract desc", __func__);
        return extract_descriptions_terminate(false);
    }
    // 公開鍵からアルゴリズムを抽出
    unsigned char alg = tool_crypto_get_algorithm_from_evp_pkey(m_public_key);
    // アルゴリズム名を設定
    switch (alg) {
        case CRYPTO_ALG_RSA2048:
            m_cert_desc.alg_name = "RSA2048";
            break;
        case CRYPTO_ALG_ECCP256:
            m_cert_desc.alg_name = "ECCP256";
            break;
        default:
            m_cert_desc.alg_name = "UNKNOWN";
            break;
    }
    // 証明書から有効期限を抽出（YYmmddHHMMSSZ 形式、13 bytes）
    const ASN1_TIME *not_after = X509_get0_notAfter(m_certificate);
    if (not_after == NULL) {
        log_debug("%s: Failed parsing ASN.1 time for extract desc", __func__);
        return extract_descriptions_terminate(false);
    }
    if (strlen((char *)not_after->data) != 13) {
        log_debug("%s: ASN.1 time format error (%s)", __func__, not_after->data);
        return extract_descriptions_terminate(false);
    }
    // YYmmddHHMMSSZ --> YY/mm/dd HH:MM:SS UTC 形式に変換
    int YY, mm, dd, HH, MM, SS;
    char c;
    sscanf((char *)not_after->data, "%02d%02d%02d%02d%02d%02d%01c", &YY, &mm, &dd, &HH, &MM, &SS, &c);
    if (c != 'Z') {
        log_debug("%s: ASN.1 time is not UTC (%s)", __func__, not_after->data);
        return extract_descriptions_terminate(false);
    }
    sprintf(m_cert_desc.not_after, "%02d/%02d/%02d %02d:%02d:%02d GMT", YY, mm, dd, HH, MM, SS);
    // 発行先を抽出
    X509_NAME *subject = X509_get_subject_name(m_certificate);
    if (subject == NULL) {
        log_debug("%s: Failed parsing subject name", __func__);
        return extract_descriptions_terminate(false);
    }
    if (X509_NAME_oneline(subject, m_cert_desc.subject, CERT_SUBJ_NAME_MAX_SIZE) == NULL) {
        log_debug("%s: Failed to get subject name", __func__);
        return extract_descriptions_terminate(false);
    }
    // 発行元を抽出
    X509_NAME *issuer = X509_get_issuer_name(m_certificate);
    if (issuer == NULL) {
        log_debug("%s: Failed parsing issuer name", __func__);
        return extract_descriptions_terminate(false);
    }
    if (X509_NAME_oneline(issuer, m_cert_desc.issuer, CERT_SUBJ_NAME_MAX_SIZE) == NULL) {
        log_debug("%s: Failed to get issuer name", __func__);
        return extract_descriptions_terminate(false);
    }
    // 証明書ハッシュ（SHA-256）を抽出
    const EVP_MD *md = EVP_sha256();
    if (md == NULL) {
        log_debug("%s: Failed to initialize SHA-256", __func__);
        return extract_descriptions_terminate(false);
    }
    unsigned int md_len = CERT_HASH_MAX_SIZE;
    if (X509_digest(m_certificate, md, m_cert_desc.hash, &md_len) == 0) {
        log_debug("%s: Failed to get SHA-256 hash of certificate", __func__);
        return extract_descriptions_terminate(false);
    }
    // 処理終了
    return extract_descriptions_terminate(true);
}

CERT_DESC *tool_crypto_certificate_extracted_descriptions(void)
{
    return &m_cert_desc;
}
