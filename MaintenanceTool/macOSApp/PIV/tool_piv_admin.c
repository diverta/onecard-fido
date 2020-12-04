//
//  tool_piv_admin.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/03.
//
#include <string.h>

#include "debug_log.h"
#include "tool_crypto_common.h"
#include "tool_crypto_certificate.h"
#include "tool_crypto_private_key.h"
#include "tool_piv_admin.h"
#include "ToolPIVCommon.h"

// デフォルトの3DES鍵
static const unsigned char default_3des_key[] = {
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
};

// 秘密鍵のアルゴリズムを保持
static unsigned char m_alg;

// 鍵／証明書のバイナリーイメージを格納
static unsigned char m_binary_data[CERTIFICATE_MAX_SIZE];
static size_t        m_binary_size;

// 変換されたTLVデータを格納
static unsigned char m_tlv_bytes[CERTIFICATE_MAX_SIZE];
static size_t        m_tlv_size;

// CCID I/F経由で転送するAPDUデータを格納
static unsigned char m_apdu_bytes[CERTIFICATE_MAX_SIZE];
static size_t        m_apdu_size;

//
// utility function
//
static size_t tlv_set_length(unsigned char *buffer, size_t length)
{
    if(length < 0x80) {
        *buffer++ = (unsigned char)length;
        return 1;
    } else if(length < 0x100) {
        *buffer++ = 0x81;
        *buffer++ = (unsigned char)length;
        return 2;
    } else {
        *buffer++ = 0x82;
        *buffer++ = (length >> 8) & 0xff;
        *buffer++ = length & 0xff;
        return 3;
    }
}

static size_t set_object_header(unsigned int object_id, unsigned char *buffer)
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
// functions for private key data
//
static size_t generate_APDU_data_rsa_2048(unsigned char *apdu_data, unsigned char *binary_data)
{
    // 変数初期化
    size_t offset = 0;
    size_t offset_b = 0;

    // P
    apdu_data[offset++] = 0x01;
    offset += tlv_set_length(apdu_data + offset, RSA2048_PQ_SIZE);
    memcpy(apdu_data + offset, binary_data + offset_b, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    offset_b += RSA2048_PQ_SIZE;

    // Q
    apdu_data[offset++] = 0x02;
    offset += tlv_set_length(apdu_data + offset, RSA2048_PQ_SIZE);
    memcpy(m_apdu_bytes + offset, binary_data + offset_b, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    offset_b += RSA2048_PQ_SIZE;

    // DP
    apdu_data[offset++] = 0x03;
    offset += tlv_set_length(apdu_data + offset, RSA2048_PQ_SIZE);
    memcpy(apdu_data + offset, binary_data + offset_b, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    offset_b += RSA2048_PQ_SIZE;

    // DQ
    apdu_data[offset++] = 0x04;
    offset += tlv_set_length(apdu_data + offset, RSA2048_PQ_SIZE);
    memcpy(apdu_data + offset, binary_data + offset_b, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    offset_b += RSA2048_PQ_SIZE;

    // QINV
    apdu_data[offset++] = 0x05;
    offset += tlv_set_length(apdu_data + offset, RSA2048_PQ_SIZE);
    memcpy(apdu_data + offset, binary_data + offset_b, RSA2048_PQ_SIZE);
    offset += RSA2048_PQ_SIZE;
    log_debug("%s: Generated RSA-2048 private key TLV for import (%d bytes)", __func__, offset);

    // APDUのサイズを戻す
    return offset;
}

static size_t generate_APDU_data_eccp_256(unsigned char *apdu_data, unsigned char *ec_pk)
{
    // 変数初期化
    size_t offset = 0;

    apdu_data[offset++] = 0x06;
    offset += tlv_set_length(apdu_data + offset, ECCP256_KEY_SIZE);
    memcpy(apdu_data + offset, ec_pk, ECCP256_KEY_SIZE);
    offset += ECCP256_KEY_SIZE;
    log_debug("%s: Generated ECCP-256 private key TLV for import (%d bytes)", __func__, offset);

    // APDUのサイズを戻す
    return offset;
}

static void generate_private_key_APDU(void)
{
    // 変数初期化
    memset(m_apdu_bytes, 0, sizeof(m_apdu_bytes));
    m_apdu_size = 0;

    switch (m_alg) {
        case CRYPTO_ALG_RSA2048:
            m_apdu_size = generate_APDU_data_rsa_2048(m_apdu_bytes, m_binary_data);
            break;
        case CRYPTO_ALG_ECCP256:
            m_apdu_size = generate_APDU_data_eccp_256(m_apdu_bytes, m_binary_data);
            break;
        default:
            break;
    }
}

//
// functions for certificate data
//
static void tool_crypto_certificate_TLV_data(void)
{
    // 変数初期化
    memset(m_tlv_bytes, 0, sizeof(m_tlv_bytes));
    size_t offset = 0;

    // data
    m_tlv_bytes[offset++] = TAG_CERT;
    offset += tlv_set_length(m_tlv_bytes + offset, m_binary_size);
    memcpy(m_tlv_bytes + offset, m_binary_data, m_binary_size);
    offset += m_binary_size;

    // compression info & LRC trailer
    m_tlv_bytes[offset++] = TAG_CERT_COMPRESS;
    m_tlv_bytes[offset++] = 0x01;
    m_tlv_bytes[offset++] = 0x00;
    m_tlv_bytes[offset++] = TAG_CERT_LRC;
    m_tlv_bytes[offset++] = 0x00;

    // TLVのサイズを保持
    m_tlv_size = offset;
}

static void generate_certificate_APDU(unsigned char key_slot_id)
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
            object_id = 0;
            break;
    }

    // 証明書バイナリーのTLVデータを先に生成
    tool_crypto_certificate_TLV_data();
    
    // object info
    size_t offset = set_object_header(object_id, m_apdu_bytes);

    // object size & data
    m_apdu_bytes[offset++] = TAG_DATA_OBJECT_VALUE;
    offset += tlv_set_length(m_apdu_bytes + offset, m_tlv_size);
    memcpy(m_apdu_bytes + offset, m_tlv_bytes, m_tlv_size);
    offset += m_tlv_size;

    // APDUのサイズを保持
    m_apdu_size = offset;
}

//
// public functions
//
unsigned char *tool_piv_admin_des_default_key(void)
{
    return (unsigned char *)default_3des_key;
}

bool tool_piv_admin_load_private_key(unsigned char key_slot_id, const char *pem_path, unsigned char *algorithm)
{
    // PEM形式の秘密鍵ファイルから、バイナリーイメージを抽出
    m_binary_size = sizeof(m_binary_data);
    if (tool_crypto_private_key_extract_from_pem(pem_path, &m_alg, m_binary_data, &m_binary_size) == false) {
        return false;
    }
    // 取得したアルゴリズムを引数領域にセット
    *algorithm = m_alg;
    // バイナリーイメージから、証明書インポート処理用のAPDUを生成
    generate_private_key_APDU();
    return true;
}

bool tool_piv_admin_load_certificate(unsigned char key_slot_id, const char *pem_path)
{
    // PEM形式の証明書ファイルから、バイナリーイメージを抽出
    m_binary_size = sizeof(m_binary_data);
    if (tool_crypto_certificate_extract_from_pem(pem_path, m_binary_data, &m_binary_size) == false) {
        return false;
    }
    // バイナリーイメージから、証明書インポート処理用のAPDUを生成
    generate_certificate_APDU(key_slot_id);
    return true;
}

unsigned char *tool_piv_admin_generated_APDU_data(void)
{
    // APDU格納領域の参照を戻す
    return m_apdu_bytes;
}

size_t tool_piv_admin_generated_APDU_size(void)
{
    // APDUのサイズを戻す
    return m_apdu_size;
}
