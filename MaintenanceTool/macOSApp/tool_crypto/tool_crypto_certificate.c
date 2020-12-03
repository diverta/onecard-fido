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
#include "tool_crypto_private_key.h"
#include "ToolPivCommon.h"

// 証明書のバイナリーイメージを格納
static unsigned char m_cert_data[CERTIFICATE_MAX_SIZE];
static size_t        m_cert_size;

// 変換されたTLVデータを格納
static unsigned char m_tlv_bytes[CERTIFICATE_MAX_SIZE];
static size_t        m_tlv_size;

// CCID I/F経由で転送するAPDUデータを格納
static unsigned char m_apdu_bytes[CERTIFICATE_MAX_SIZE];
static size_t        m_apdu_size;

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
// utility function
//
static size_t tool_crypto_set_object_header(unsigned int object_id, unsigned char *buffer)
{
    size_t offset = 0;
    buffer[offset++] = TAG_DATA_OBJECT;

    // オブジェクト長の情報を設定
    buffer[offset++] = 3;
    buffer[offset++] = (object_id >> 16) & 0xff;
    buffer[offset++] = (object_id >> 8) & 0xff;
    buffer[offset++] = object_id & 0xff;

    // 設定した情報の長さを戻す
    return offset;
}

//
// public functions
//
bool tool_crypto_certificate_extract_from_pem(const char *pem_path)
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
    if (cert_len > sizeof(m_cert_data) || cert_len < 0) {
        log_debug("%s: Length of certificate (%d bytes) is more than can fit", __func__, cert_len);
        return extract_from_pem_terminate(false);
    }
    // 証明書バイナリーイメージを配列に抽出
    unsigned char *p_cert_data = m_cert_data;
    int cert_loaded = i2d_X509(m_certificate, &p_cert_data);
    if (cert_loaded != cert_len) {
        log_debug("%s: Failed loading certificate (size=%d, loaded=%d)", __func__, cert_len, cert_loaded);
        return extract_from_pem_terminate(false);
    }
    // 処理終了
    m_cert_size = (size_t)cert_loaded;
    return extract_from_pem_terminate(true);
}

static void tool_crypto_certificate_TLV_data(void)
{
    // 変数初期化
    memset(m_tlv_bytes, 0, sizeof(m_tlv_bytes));
    size_t offset = 0;

    // data
    m_tlv_bytes[offset++] = TAG_CERT;
    offset += tool_crypto_tlv_set_length(m_tlv_bytes + offset, m_cert_size);
    memcpy(m_tlv_bytes + offset, m_cert_data, m_cert_size);
    offset += m_cert_size;

    // compression info & LRC trailer
    m_tlv_bytes[offset++] = TAG_CERT_COMPRESS;
    m_tlv_bytes[offset++] = 0x01;
    m_tlv_bytes[offset++] = 0x00;
    m_tlv_bytes[offset++] = TAG_CERT_LRC;
    m_tlv_bytes[offset++] = 0x00;

    // TLVのサイズを保持
    m_tlv_size = offset;
}

unsigned char *tool_crypto_certificate_APDU_data(unsigned char key_slot_id)
{
    // 変数初期化
    memset(m_apdu_bytes, 0, sizeof(m_apdu_bytes));
    m_apdu_size = 0;

    // get object ID
    unsigned int object_id;
    switch (key_slot_id) {
        case PIV_KEY_AUTHENTICATION:
            object_id = PIV_OBJ_AUTHENTICATION;
            break;
        case PIV_KEY_SIGNATURE:
            object_id = PIV_OBJ_SIGNATURE;
            break;
        case PIV_KEY_KEYMGM:
            object_id = PIV_OBJ_KEY_MANAGEMENT;
            break;
        default:
            return m_apdu_bytes;
    }

    // 証明書バイナリーのTLVデータを先に生成
    tool_crypto_certificate_TLV_data();
    
    // object info
    size_t offset = tool_crypto_set_object_header(object_id, m_apdu_bytes);

    // object size & data
    m_tlv_bytes[offset++] = TAG_DATA_OBJECT_VALUE;
    offset += tool_crypto_tlv_set_length(m_apdu_bytes + offset, m_tlv_size);
    memcpy(m_apdu_bytes + offset, m_tlv_bytes, m_tlv_size);
    offset += m_tlv_size;

    // APDUのサイズを保持
    m_apdu_size = offset;

    // 先頭アドレスを戻す
    return m_apdu_bytes;
}

size_t tool_crypto_certificate_APDU_size(void)
{
    // APDUのサイズを戻す
    return m_apdu_size;
}
