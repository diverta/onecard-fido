//
//  oath_util.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/16.
//
#include <string.h>
#include <time.h>

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

bool generate_account_add_apdu(const char *account, size_t account_size, const char *base32_secret, size_t base32_secret_size)
{
    // Secret（Base32暗号テキスト）をバイト配列化
    uint8_t decoded[base32_secret_size];
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

static void convert_uint64_to_be_bytes(uint64_t ui, uint8_t *b, int offset)
{
    uint8_t *s = (uint8_t *)(&ui);
    for (int i = 0; i < sizeof(ui); i++) {
        b[i + offset] = s[sizeof(ui) - 1 - i];
    }
}

bool generate_apdu_for_calculate(const char *account, size_t account_size)
{
    // 現在のUNIX時刻を取得
    time_t t = time(NULL);
    uint64_t now_epoch_seconds = (uint64_t)t;
    uint64_t challenge = now_epoch_seconds / 30;
    // Challenge（Unix時間）をビッグエンディアンでバイト配列化
    uint8_t challenge_bytes[8];
    size_t  challenge_size = sizeof(challenge_bytes);
    convert_uint64_to_be_bytes(challenge, challenge_bytes, 0);
    // 変数初期化
    apdu_size = 2 + account_size + 2 + challenge_size;
    int offset = 0;
    // アカウント
    apdu_bytes[offset++] = 0x71;
    apdu_bytes[offset++] = account_size;
    // アカウントをコピー
    memcpy(apdu_bytes + offset, account, account_size);
    offset += account_size;
    // Challenge
    apdu_bytes[offset++] = 0x74;
    apdu_bytes[offset++] = challenge_size;
    // Challengeをコピー
    memcpy(apdu_bytes + offset, challenge_bytes, account_size);
    return true;
}
