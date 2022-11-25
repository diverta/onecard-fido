/* 
 * File:   sha1.c
 * Author: makmorit
 *
 * Created on 2022/11/24, 18:37
 */
#include <stdint.h>
#include <stddef.h>

#include "sdk_common.h"

// for logging informations
#define NRF_LOG_MODULE_NAME sha1
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

struct sha1_context {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
};
typedef struct sha1_context SHA1_CTX;

static void sha1_init(SHA1_CTX* context)
{
    // SHA1 initialization constants
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

void sha1_hash_calculate(size_t num_elem, uint8_t *addr[], size_t *len, uint8_t *mac)
{
    // TODO: 仮の実装です。
    SHA1_CTX ctx;
    sha1_init(&ctx);
}
