//
//  tool_piv_admin.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/03.
//
#include <stdlib.h>
#include <string.h>

// for obtaining current time
#include <time.h>

#include "debug_log.h"
#include "tool_crypto_common.h"
#include "tool_crypto_certificate.h"
#include "tool_crypto_private_key.h"
#include "tool_piv_admin.h"
#include "ToolPIVCommon.h"

//
// CHUIDテンプレート
//  0x30: FASC-N
//  0x34: Card UUID / GUID
//  0x35: Exp. Date
//  0x3e: Signature
//  0xfe: Error Detection Code
//
static const unsigned char chuid_template[] = {
    0x30, 0x19, 0xd4, 0xe7, 0x39, 0xda, 0x73, 0x9c, 0xed, 0x39, 0xce, 0x73, 0x9d, 0x83, 0x68, 0x58, 0x21, 0x08, 0x42, 0x10, 0x84, 0x21, 0xc8, 0x42, 0x10, 0xc3, 0xeb,
    0x34, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x35, 0x08, 0x32, 0x30, 0x33, 0x30, 0x30, 0x31, 0x30, 0x31,
    0x3e, 0x00,
    0xfe, 0x00
};

//
// CCCテンプレート
//  f0: Card Identifier
//      0xa000000116 == GSC-IS RID
//      0xff == Manufacturer ID (dummy)
//      0x02 == Card type (javaCard)
//      next 14 bytes: card ID
static const unsigned char ccc_template[] = {
    0xf0, 0x15,
    0xa0, 0x00, 0x00, 0x01, 0x16,
    0xff,
    0x02,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xf1, 0x01, 0x21,
    0xf2, 0x01, 0x21,
    0xf3, 0x00,
    0xf4, 0x01, 0x00,
    0xf5, 0x01, 0x10,
    0xf6, 0x00,
    0xf7, 0x00,
    0xfa, 0x00,
    0xfb, 0x00,
    0xfc, 0x00,
    0xfd, 0x00,
    0xfe, 0x00
};

// テンプレート内項目の設定位置、長さ
#define PIV_CARDID_OFFSET   29
#define PIV_CARDID_SIZE     16
#define PIV_CARDEXP_OFFSET  47
#define PIV_CARDEXP_SIZE    8
#define PIV_CCCID_OFFSET    9
#define PIV_CCCID_SIZE      14

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
    if (length < 0x80) {
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

static size_t tlv_get_length(unsigned char *buffer, size_t size, size_t *val_len)
{
    // 不正なTLVの場合、戻り値を０に設定します
    if (buffer[0] < 0x80 && 1 < size) {
        *val_len = buffer[0];
        return (1 + *val_len <= size) ? 1 : 0;
    } else if (buffer[0] == 0x81 && 2 < size) {
        *val_len = buffer[1];
        return (2 + *val_len <= size) ? 2 : 0;
    } else if (buffer[0] == 0x82 && 3 < size) {
        *val_len = ((buffer[1] << 8) & 0xff00) + (buffer[2] & 0x00ff);
        return (3 + *val_len <= size) ? 3 : 0;
    }
    *val_len = 0;
    return 0;
}

size_t tool_piv_admin_set_object_header(unsigned int object_id, unsigned char *buffer)
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
    size_t offset = tool_piv_admin_set_object_header(object_id, m_apdu_bytes);

    // object size & data
    m_apdu_bytes[offset++] = TAG_DATA_OBJECT_VALUE;
    offset += tlv_set_length(m_apdu_bytes + offset, m_tlv_size);
    memcpy(m_apdu_bytes + offset, m_tlv_bytes, m_tlv_size);
    offset += m_tlv_size;

    // APDUのサイズを保持
    m_apdu_size = offset;
}

//
// functions for ID data
//
static void set_random_bytes_to_field(unsigned char *buf, size_t size)
{
    // ランダムなバイトデータを設定
    for (int i = 0; i < size; i++) {
        uint32_t r = arc4random_uniform(0xff);
        buf[i] = (unsigned char)r;
    }
}

static void set_random_number_to_field(char *buf, size_t size)
{
    // ランダムな半角数字を設定
    char char_set[] = "0123456789";
    for (int i = 0; i < size; i++) {
        int j = rand() % sizeof(char_set);
        buf[i] = char_set[j];
    }
}

static void set_next_year_to_field(char *buf, size_t size)
{
    // 現在の１年後の日付を設定
    time_t nowTime = time(NULL);
    struct tm *t = localtime(&nowTime);
    t->tm_year++;
    strftime(buf, size, "%Y%m%d", t);
}

static void generate_CHUID_bytes(unsigned char *buf, size_t *size)
{
    // 生成されるCHUID格納領域の長さ
    size_t s = sizeof(chuid_template);
    // テンプレートの内容をコピー
    memcpy(buf, chuid_template, s);
    // ランダムなIDを設定
    set_random_bytes_to_field(buf + PIV_CARDID_OFFSET, PIV_CARDID_SIZE);
    // 有効期限を１年後に設定
    set_next_year_to_field((char *)buf + PIV_CARDEXP_OFFSET, PIV_CARDEXP_SIZE);
    // データ長を設定
    *size = s;
}

static void generate_CCC_bytes(unsigned char *buf, size_t *size)
{
    // 生成されるCHUID格納領域の長さ
    size_t s = sizeof(ccc_template);
    // テンプレートの内容をコピー
    memcpy(buf, ccc_template, s);
    // ランダムなIDを設定
    set_random_number_to_field((char *)buf + PIV_CCCID_OFFSET, PIV_CCCID_SIZE);
    // データ長を設定
    *size = s;
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

unsigned char *tool_piv_admin_generate_CHUID_APDU(size_t *size)
{
    // 変数初期化
    memset(m_apdu_bytes, 0, sizeof(m_apdu_bytes));

    // CHUIDのバイナリーデータを先に生成
    generate_CHUID_bytes(m_binary_data, &m_binary_size);
    
    // object info
    size_t offset = tool_piv_admin_set_object_header(PIV_OBJ_CHUID, m_apdu_bytes);

    // object size & data
    m_apdu_bytes[offset++] = TAG_DATA_OBJECT_VALUE;
    offset += tlv_set_length(m_apdu_bytes + offset, m_binary_size);
    memcpy(m_apdu_bytes + offset, m_binary_data, m_binary_size);
    offset += m_binary_size;

    // CHUID格納領域の参照を戻す
    *size = offset;
    return m_apdu_bytes;
}

unsigned char *tool_piv_admin_generate_CCC_APDU(size_t *size)
{
    // 変数初期化
    memset(m_apdu_bytes, 0, sizeof(m_apdu_bytes));

    // CCCのバイナリーデータを先に生成
    generate_CCC_bytes(m_binary_data, &m_binary_size);
    
    // object info
    size_t offset = tool_piv_admin_set_object_header(PIV_OBJ_CAPABILITY, m_apdu_bytes);

    // object size & data
    m_apdu_bytes[offset++] = TAG_DATA_OBJECT_VALUE;
    offset += tlv_set_length(m_apdu_bytes + offset, m_binary_size);
    memcpy(m_apdu_bytes + offset, m_binary_data, m_binary_size);
    offset += m_binary_size;

    // CCC格納領域の参照を戻す
    *size = offset;
    return m_apdu_bytes;
}

bool tool_piv_admin_extract_cert_from_TLV(unsigned char *buffer, size_t size)
{
    //
    // 証明書データを格納しているTLVから、証明書データだけを抽出します。
    //   TLV: 538203957082038cXXXX...XXXX710100fe00 (921 bytes)
    //   --> TLV data: 7082038cXXXX...XXXX710100fe00
    //       TLV size: 0x0395 (917 bytes)
    //   --> cert data: XXXX...XXXX
    //       cert size: 0x038c (908 bytes)
    //
    // 領域を初期化
    memset(m_binary_data, 0, sizeof(m_binary_data));
    m_binary_size = 0;
    // 証明書データを格納しているTLVを抽出
    unsigned char *tlv_data = buffer + 1;
    size_t tlv_size = size - 1;
    size_t obj_len;
    size_t offset_obj = tlv_get_length(tlv_data, tlv_size, &obj_len);
    if (offset_obj == 0) {
        // 不正なTLVの場合は終了
        log_debug("%s: Invalid TLV (object length)", __func__);
        return false;
    }
    // 証明書データを抽出
    tlv_data += offset_obj + 1;
    size_t val_len;
    size_t offset_val = tlv_get_length(tlv_data, obj_len, &val_len);
    if (offset_val == 0) {
        // 不正なTLVの場合は終了
        log_debug("%s: Invalid TLV (object value length)", __func__);
        return false;
    }
    tlv_data += offset_val;
    // 抽出した証明書データを内部保持
    memcpy(m_binary_data, tlv_data, val_len);
    m_binary_size = val_len;
    return true;
}

unsigned char *tool_piv_admin_extracted_cert_data(void)
{
    // 証明書データ格納領域の参照を戻す
    return m_binary_data;
}

size_t tool_piv_admin_extracted_cert_size(void)
{
    // 証明書データのサイズを戻す
    return m_binary_size;
}
