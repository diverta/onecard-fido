//
//  aes_256_cbc.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#import <CommonCrypto/CommonCryptor.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

bool aes_256_cbc_enc(uint8_t *key, uint8_t *plain, size_t plain_size, uint8_t *encoded, size_t *encoded_size)
{
    bool   ret = false;
    size_t key_size = kCCKeySizeAES256;

    // Allocate buffer for encryption
    size_t size = plain_size + kCCBlockSizeAES128;
    void  *buff = malloc(size);

    // Perform encryption
    CCCryptorStatus result = CCCrypt(kCCEncrypt, kCCAlgorithmAES, 0, key, key_size, NULL, plain, plain_size, buff, size, encoded_size);
    // Copy encrypted bytes
    if (result == kCCSuccess) {
        memcpy(encoded, buff, *encoded_size);
        ret = true;
    }

    // Deallocate buffer
    free(buff);
    return ret;
}

bool aes_256_cbc_dec(uint8_t *key, uint8_t *encoded, size_t encoded_size, uint8_t *decoded, size_t *decoded_size)
{
    bool   ret = false;
    size_t key_size = kCCKeySizeAES256;

    // Allocate buffer for decryption
    size_t size = encoded_size + kCCBlockSizeAES128;
    void  *buff = malloc(size);

    // Perform decryption
    CCCryptorStatus result = CCCrypt(kCCDecrypt, kCCAlgorithmAES, 0, key, key_size, NULL, encoded, encoded_size, buff, size, decoded_size);
    // Copy decrypted bytes
    if (result == kCCSuccess) {
        memcpy(decoded, buff, *decoded_size);
        ret = true;
    }

    // Deallocate buffer
    free(buff);
    return ret;
}
