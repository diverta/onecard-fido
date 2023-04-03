//
//  CryptoTestCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#import <CommonCrypto/CommonCryptor.h>
#import "aes_256_cbc.h"
#import "triple_des.h"
#import "ToolLogFile.h"

// on migrating
#import "debug_log.h"
#import "AES256CBC.h"
#import "CryptoTestCommand.h"

// for des
#import "tool_piv_admin.h"
#import "tool_crypto_des.h"

@interface CryptoTestCommand ()

@end

@implementation CryptoTestCommand
    
    - (void)testAES256CBC {
        NSData *data = [@"This is a sample string.12345678" dataUsingEncoding:NSUTF8StringEncoding];

        uint8_t *data_bytes = (uint8_t *)[data bytes];
        size_t data_size = [data length];

        uint8_t key[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x11,
            0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
            0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
        };

        [[ToolLogFile defaultLogger] debugWithFormat:@"sample data (%d bytes)", data_size];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:data_bytes size:data_size];

        size_t encoded_size = data_size + kCCBlockSizeAES128;
        uint8_t encoded[encoded_size];

        if (aes_256_cbc_enc(key, data_bytes, data_size, encoded, &encoded_size) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes_256_cbc_enc: %s", log_debug_message()];
            return;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"encoded data (%d bytes)", encoded_size];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:encoded size:encoded_size];

        size_t decoded_size = data_size + kCCBlockSizeAES128;
        uint8_t decoded[encoded_size];

        if (aes_256_cbc_dec(key, encoded, encoded_size, decoded, &decoded_size) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes_256_cbc_dec: %s", log_debug_message()];
            return;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"decoded data (%d bytes)", decoded_size];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:decoded size:decoded_size];

        // 移行前の処理
        fido_blob_t *pkey = fido_blob_new();
        fido_blob_t *ppin = fido_blob_new();
        fido_blob_t *pe   = fido_blob_new();
        fido_blob_t *pd   = fido_blob_new();
        fido_blob_set(pkey, key, 32);
        fido_blob_set(ppin, data_bytes, data_size);

        if (aes256_cbc_enc(pkey, ppin, pe) != 0) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes256_cbc_enc: %s", log_debug_message()];
            goto fail;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"aes256_cbc_enc encoded data (%d bytes)", pe->len];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:pe->ptr size:pe->len];

        if (aes256_cbc_dec(pkey, pe, pd) != 0) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes256_cbc_dec: %s", log_debug_message()];
            goto fail;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"aes256_cbc_dec decoded data (%d bytes)", pd->len];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:pd->ptr size:pd->len];

    fail:
        fido_blob_free(&ppin);
        fido_blob_free(&pkey);
        fido_blob_free(&pe);
        fido_blob_free(&pd);
    }

    - (void)testTripleDES {
        uint8_t *pw = tool_piv_admin_des_default_key();

        // for research
        [[ToolLogFile defaultLogger] debugWithFormat:@"DES key (%d bytes)", kCCKeySize3DES];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:pw size:kCCKeySize3DES];

        // 為念で３回テスト
        uint8_t encrypted1[] = {
            0xb3, 0xd8, 0xb3, 0x50, 0xfd, 0x75, 0x1b, 0x19
        };
        [self testTripleDESWithPW:pw withBytes:encrypted1 withSize:sizeof(encrypted1)];

        uint8_t encrypted2[] = {
            0xed, 0x41, 0x93, 0x13, 0xe0, 0xd9, 0x72, 0x81
        };
        [self testTripleDESWithPW:pw withBytes:encrypted2 withSize:sizeof(encrypted2)];
        
        uint8_t encrypted3[] = {
            0x29, 0x95, 0x48, 0xbe, 0x6f, 0x1a, 0xab, 0xd6
        };
        [self testTripleDESWithPW:pw withBytes:encrypted3 withSize:sizeof(encrypted3)];
    }

    - (void)testTripleDESWithPW:(uint8_t *)pw withBytes:(uint8_t *)encrypted withSize:(size_t)size {
        uint8_t decrypted[kCCKeySizeDES];
        size_t decryptedSize = sizeof(decrypted);

        [[ToolLogFile defaultLogger] debugWithFormat:@"DES encrypted (%d bytes)", size];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:encrypted size:size];

        // 移行後の処理
        if (triple_des_import_key(pw, kCCKeySize3DES) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"triple_des_import_key: %s", log_debug_message()];
            return;
        }
        memset(decrypted, 0, kCCKeySizeDES);
        if (triple_des_decrypt(encrypted, sizeof(encrypted), decrypted, &decryptedSize) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"triple_des_import_key: %s", log_debug_message()];
            return;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"triple_des_decrypt (%d bytes)", decryptedSize];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:decrypted size:decryptedSize];

        // 移行前の処理
        if (tool_crypto_des_import_key(pw, kCCKeySize3DES) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"tool_crypto_des_import_key: %s", log_debug_message()];
            return;
        }
        memset(decrypted, 0, kCCKeySizeDES);
        if (tool_crypto_des_decrypt(encrypted, sizeof(encrypted), decrypted, &decryptedSize) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"tool_crypto_des_decrypt: %s", log_debug_message()];
            return;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"tool_crypto_des_decrypt (%d bytes)", decryptedSize];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:decrypted size:decryptedSize];
    }

@end
