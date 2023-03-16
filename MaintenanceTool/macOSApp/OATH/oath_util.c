//
//  oath_util.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/16.
//
#include <string.h>

#include "base32_util.h"
#include "oath_util.h"

static uint8_t apdu_bytes[512];
static size_t  apdu_size;

uint8_t *generated_oath_apdu_bytes(void)
{
    return apdu_bytes;
}

size_t generated_oath_apdu_size(void)
{
    return apdu_size;
}

bool generate_account_add_apdu(const char *account, const char *base32_secret)
{
    // Account長
    size_t account_size = strlen(account);
    // Secret（Base32暗号テキスト）をバイト配列化
    size_t  encoded_size = strlen(base32_secret);
    uint8_t decoded[encoded_size];
    size_t  decoded_size = sizeof(decoded);
    if (base32_decode(decoded, &decoded_size, base32_secret) == false) {
        return false;
    }
    // 変数初期化
    apdu_size = 2 + account_size + 4 + decoded_size;
    int offset = 0;
    // アカウント
    apdu_bytes[offset++] = 0x71;
    apdu_bytes[offset++] = account_size;
    // アカウントをコピー
    memcpy(apdu_bytes + offset, account, account_size);
    offset += account_size;
    // Secret
    apdu_bytes[offset++] = 0x73;
    apdu_bytes[offset++] = decoded_size + 2;
    apdu_bytes[offset++] = 0x21;
    apdu_bytes[offset++] = 0x06;
    // Secretをコピー
    memcpy(apdu_bytes + offset, decoded, decoded_size);
    return true;
}
