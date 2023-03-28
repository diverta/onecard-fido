//
//  CryptoTestCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/28.
//
#import <CommonCrypto/CommonCryptor.h>
#import "aes_256_cbc.h"
#import "ToolLogFile.h"

// on migrating
#import "debug_log.h"
#import "AES256CBC.h"
#import "CryptoTestCommand.h"

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
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes_256_cbc_enc fail"];
            return;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"encoded data (%d bytes)", encoded_size];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:encoded size:encoded_size];

        size_t decoded_size = data_size + kCCBlockSizeAES128;
        uint8_t decoded[encoded_size];

        if (aes_256_cbc_dec(key, encoded, encoded_size, decoded, &decoded_size) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes_256_cbc_dec fail"];
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
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes256_cbc_enc fail: %s", log_debug_message()];
            goto fail;
        }

        [[ToolLogFile defaultLogger] debugWithFormat:@"aes256_cbc_enc encoded data (%d bytes)", pe->len];
        [[ToolLogFile defaultLogger] hexdumpOfBytes:pe->ptr size:pe->len];

        if (aes256_cbc_dec(pkey, pe, pd) != 0) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"aes256_cbc_dec fail: %s", log_debug_message()];
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

@end
