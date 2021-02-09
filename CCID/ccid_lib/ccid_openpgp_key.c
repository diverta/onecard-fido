/* 
 * File:   ccid_openpgp_key.c
 * Author: makmorit
 *
 * Created on 2021/02/09, 16:20
 */
#include <string.h>

#include "ccid_openpgp.h"
#include "ccid_openpgp_key.h"

//
// Keys for OpenPGP
//
#define KEY_TYPE_RSA                0x01

//
// offset
//  1: Reserved for length of modulus, default: 2048
//  3: length of exponent: 32 bit
//
static uint8_t rsa_attr[] = {KEY_TYPE_RSA, 0x08, 0x00, 0x00, 0x20, 0x00};
static uint8_t key_fingerprint[KEY_FINGERPRINT_LENGTH];
static uint8_t key_datetime[KEY_DATETIME_LENGTH];

uint16_t openpgp_key_get_attributes(uint8_t tag, uint8_t *buf, size_t *size) 
{
    // TODO: 仮の実装です。
    (void)tag;
    memcpy(buf, rsa_attr, sizeof(rsa_attr));
    *size = sizeof(rsa_attr);

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_key_get_fingerprint(uint8_t tag, void *buf, size_t *size)
{
    // TODO: 仮の実装です。
    (void)tag;
    memcpy(buf, key_fingerprint, sizeof(key_fingerprint));
    *size = sizeof(key_fingerprint);
    
    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_key_get_datetime(uint8_t tag, void *buf, size_t *size)
{
    // TODO: 仮の実装です。
    (void)tag;
    memcpy(buf, key_datetime, sizeof(key_datetime));
    *size = sizeof(key_datetime);

    // 正常終了
    return SW_NO_ERROR;
}

uint16_t openpgp_key_get_status(uint8_t tag, uint8_t *status)
{
    // TODO: 仮の実装です。
    (void)tag;
    *status = 0;

    // 正常終了
    return SW_NO_ERROR;
}
