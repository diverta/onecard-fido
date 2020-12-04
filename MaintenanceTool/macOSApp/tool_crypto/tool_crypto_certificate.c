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

//
// references for memory
//
static FILE        *m_input_file = NULL;
static X509        *m_certificate = NULL;

static void initialize_references(void)
{
    m_certificate = NULL;
    m_input_file = NULL;
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
