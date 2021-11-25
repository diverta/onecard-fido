#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "fido_command_common.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(u2f_signature);
#endif

// for debug data
#define DEBUG_VERIFY_SIGN false

// ハッシュ化データに関する情報
static uint8_t hash_digest[SHA_256_HASH_SIZE];

// ASN.1形式に変換された署名を格納する領域の大きさ
#define ASN1_SIGNATURE_MAXLEN 72

#define ASN_INT 0x02;
#define ASN_SEQUENCE 0x30;

// 署名ベースおよび署名を編集するための作業領域（固定長）
#define SIGNATURE_BASE_BUFFER_LENGTH 384
static uint8_t signature_data_buffer[SIGNATURE_BASE_BUFFER_LENGTH];
static size_t  signature_data_size;

uint8_t *u2f_signature_data_buffer(void)
{
    return signature_data_buffer;
}

size_t u2f_signature_data_size(void)
{
    return signature_data_size;
}

void u2f_signature_base_data_size_set(size_t size)
{
    signature_data_size = size;
}

void u2f_signature_generate_hash_for_sign(void)
{
    // 署名対象バイト配列からSHA256アルゴリズムにより、
    // ハッシュデータ作成
    uint8_t *signature_base_buffer = signature_data_buffer;
    uint16_t signature_base_buffer_length = signature_data_size;

#if DEBUG_VERIFY_SIGN
    // for debug
    fido_log_debug("signature_base_buffer: %d bytes", signature_base_buffer_length);
    fido_log_print_hexdump_debug(signature_base_buffer, signature_base_buffer_length);
#endif

    size_t digest_size = sizeof(hash_digest);
    fido_command_calc_hash_sha256(signature_base_buffer, signature_base_buffer_length, hash_digest, &digest_size);
}

uint8_t *u2f_signature_hash_for_sign(void)
{
    return hash_digest;
}

bool u2f_signature_convert_to_asn1(uint8_t *p_signature_value)
{
    // 格納領域を確保
    uint8_t *asn1_signature = signature_data_buffer;
    if (asn1_signature == NULL) {
        fido_log_debug("u2f_crypto_create_asn1_signature: allocation failed ");
        return false;
    }
    fido_log_debug("Create ASN.1 signature start ");

    // 格納領域を初期化
    memset(asn1_signature, 0, ASN1_SIGNATURE_MAXLEN);

    // 署名データのr部、s部の先頭バイトが
    // 0b80 以上であれば、
    // その直前に 0x00 を挿入する必要がある
    // (MSBを参照して判定)
    int part_length = 32;
    uint8_t *rbytes = p_signature_value;
    uint8_t *sbytes = p_signature_value + part_length;
    
    int rbytes_leading = 0;
    if (rbytes[0] & 0x80) {
        rbytes_leading = 1;
    }

    int sbytes_leading = 0;
    if (sbytes[0] & 0x80) {
        sbytes_leading = 1;
    }

    // 署名データのヘッダー部を格納
    int total_length = 4 + rbytes_leading + part_length 
                         + sbytes_leading + part_length;
    int i = 0;
    asn1_signature[i++] = ASN_SEQUENCE;
    asn1_signature[i++] = total_length;

    // 署名データのr部を格納
    asn1_signature[i++] = ASN_INT;
    asn1_signature[i++] = rbytes_leading + part_length;
    if (rbytes_leading == 1) {
        asn1_signature[i++] = 0x00;
    }
    for (int j = 0; j < part_length; j++) {
        asn1_signature[i++] = rbytes[j];
    }

    // 署名データのs部を格納
    asn1_signature[i++] = ASN_INT;
    asn1_signature[i++] = sbytes_leading + part_length;
    if (sbytes_leading == 1) {
        asn1_signature[i++] = 0x00;
    }
    for (int k = 0; k < part_length; k++) {
        asn1_signature[i++] = sbytes[k];
    }

    // 生成されたASN.1形式署名の
    // サイズを構造体に設定
    signature_data_size = i;

    fido_log_debug("Create ASN.1 signature end ");
    return true;
}
