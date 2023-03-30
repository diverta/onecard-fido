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
#import "ToolSecurity.h"

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

    - (void)testECKey {
        // SAMPLE: 公開鍵は証明書（CRT）から、秘密鍵はPEMから抽出したものを使用
        uint8_t public_key[] = {
            0x90,0x99,0x23,0x34,0xd1,0xdb,0x65,0x29,0xf3,0xda,0x89,0x52,0x42,0x33,0xfb,0xf6,
            0x3c,0xa5,0x25,0x02,0xe3,0x41,0x91,0xe8,0xa5,0x16,0x6f,0x3f,0xcc,0x32,0xf3,0x3b,
            0x30,0xbd,0x98,0xbb,0x85,0x6f,0x75,0x85,0x05,0xe7,0x81,0x4e,0x19,0x57,0x65,0xc2,
            0xdf,0x9b,0xbe,0x2b,0x20,0x7a,0xe8,0xaa,0x21,0xb2,0xbd,0xfc,0x1b,0x13,0x77,0x1f,
        };
        uint8_t skey_bytes[] = {
            0xcf, 0x91, 0x6d, 0x82, 0x30, 0x03, 0xcb, 0x4b, 0x4d, 0xc8, 0x6a, 0xff, 0x05, 0x14, 0x49, 0xc1,
            0xf4, 0x11, 0xfc, 0x67, 0x37, 0xb7, 0x3a, 0x71, 0x53, 0xa3, 0x4e, 0x65, 0x0d, 0x03, 0x95, 0xd0,
        };
        // Securityフレームワークで処理できる形式（0x04 || X || Y || K）に変換
        NSData *privkeyData = [ToolSecurity generatePrivkeyDataFromPrivkeyBytes:skey_bytes withPubkeyBytes:public_key];
        [[ToolLogFile defaultLogger] debugWithFormat:@"EC key data for restore (%d bytes)", [privkeyData length]];
        [[ToolLogFile defaultLogger] hexdump:privkeyData];
        // EC秘密鍵を内部形式に変換
        id privSecKeyRef = [ToolSecurity generatePrivkeyFromData:privkeyData];
        if (privSecKeyRef == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"privSecKeyRef: %@", privSecKeyRef];
        // EC公開鍵を内部形式に変換
        id pubSecKeyRef = [ToolSecurity generatePubkeyFromPrivkey:privSecKeyRef];
        if (pubSecKeyRef == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"pubSecKeyRef: %@", pubSecKeyRef];
        // サンプルの署名ベース
        uint8_t sample[] = {
            0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
            0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0f, 0x10, 0x11,
            0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
            0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21,
        };
        NSData* data2sign = [[NSData alloc] initWithBytes:sample length:sizeof(sample)];
        // 署名を生成
        SecKeyAlgorithm algorithm = kSecKeyAlgorithmECDSASignatureMessageX962SHA256;
        NSData* signature = [ToolSecurity createECDSASignatureWithData:data2sign withPrivkeyRef:privSecKeyRef withAlgorithm:algorithm];
        if (signature == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"ECDSA signature (%d bytes)", [signature length]];
        [[ToolLogFile defaultLogger] hexdump:signature];
        // 署名を検証
        if ([ToolSecurity verifyECDSASignature:signature withDataToSign:data2sign withPubkeyRef:pubSecKeyRef withAlgorithm:algorithm]) {
            [[ToolLogFile defaultLogger] info:@"ECDSA signature verify success"];
        }
    }
    
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
