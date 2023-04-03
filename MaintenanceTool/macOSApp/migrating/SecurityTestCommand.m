//
//  SecurityTestCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/04/03.
//
#import "ToolLogFile.h"
#import "ToolSecurity.h"

// on migrating
#import "debug_log.h"
#import "SecurityTestCommand.h"

@interface SecurityTestCommand ()

@end

@implementation SecurityTestCommand

    - (void)testECDHWithSample {
        // SAMPLE
        uint8_t skey_bytes[] = {
            0xdd, 0x63, 0x16, 0x4b, 0x27, 0xc0, 0xed, 0xdc, 0x68, 0x5c, 0xca, 0x3b, 0x05, 0x14, 0xf6, 0x7f,
            0xa5, 0x3e, 0x5f, 0x11, 0xb0, 0xa2, 0xa1, 0x41, 0x1f, 0x07, 0xb6, 0x22, 0x15, 0xa2, 0x70, 0x1b,
        };
        uint8_t public_key[] = {
            0xf2, 0xa1, 0x33, 0x3f, 0x19, 0x85, 0x28, 0x21, 0xe8, 0x23, 0xd0, 0xac, 0x3e, 0xe0, 0xfe, 0x74,
            0x71, 0xae, 0xc2, 0x91, 0x1c, 0x94, 0xa7, 0xb0, 0xe4, 0x0e, 0x58, 0x7b, 0x17, 0x0f, 0x04, 0xb5,
            0x3b, 0x03, 0x34, 0x46, 0x53, 0x2e, 0x3e, 0xa3, 0x04, 0x39, 0xc4, 0x76, 0xb5, 0x51, 0xce, 0x99,
            0x84, 0xd1, 0x7d, 0x11, 0xfb, 0x6d, 0xf6, 0x9e, 0x00, 0xe4, 0x19, 0x19, 0xe1, 0xad, 0xb2, 0x22,
        };
        // Securityフレームワークで処理できる形式（0x04 || X || Y || K）に変換
        NSData *privkeyData = [ToolSecurity generatePrivkeyDataFromPrivkeyBytes:skey_bytes withPubkeyBytes:public_key];
        [[ToolLogFile defaultLogger] debugWithFormat:@"Host key data for restore (%d bytes)", [privkeyData length]];
        [[ToolLogFile defaultLogger] hexdump:privkeyData];
        // EC秘密鍵を内部形式に変換
        id privSecKeyRef = [ToolSecurity generatePrivkeyFromData:privkeyData];
        if (privSecKeyRef == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"privSecKeyRef: %@", privSecKeyRef];
        // SAMPLE
        uint8_t client_public_key[] = {
            0x5b, 0x7f, 0xa1, 0x9f, 0x1f, 0x30, 0xb7, 0xdc, 0x71, 0xb7, 0x4e, 0x0b, 0x4e, 0x1f, 0x18, 0x2b,
            0x95, 0x71, 0x49, 0x52, 0x41, 0x3a, 0x5e, 0x4a, 0xc7, 0x5e, 0x27, 0x07, 0xc5, 0xd2, 0x3e, 0x25,
            0x38, 0x27, 0x0c, 0x54, 0x78, 0x59, 0x89, 0xaf, 0xfe, 0x11, 0xfa, 0x80, 0x46, 0x76, 0xcc, 0xf9,
            0x08, 0x34, 0x7c, 0xbb, 0x05, 0x5f, 0xb9, 0x94, 0xe1, 0x4c, 0xef, 0x0d, 0x85, 0xe9, 0x4b, 0x69,
        };
        // Securityフレームワークで処理できる形式（0x04 || X || Y）に変換
        NSData *clientkeyData = [ToolSecurity generatePubkeyDataFromPubkeyBytes:client_public_key];
        [[ToolLogFile defaultLogger] debugWithFormat:@"Client key data for restore (%d bytes)", [clientkeyData length]];
        [[ToolLogFile defaultLogger] hexdump:clientkeyData];
        // EC公開鍵を内部形式に変換
        id pubSecKeyRef = [ToolSecurity generatePubkeyFromData:clientkeyData];
        if (pubSecKeyRef == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"clientPubSecKeyRef: %@", pubSecKeyRef];
        // 共通鍵を生成
        NSData *exchangedKey1 = [ToolSecurity generateECDHSharedSecretWithPrivate:privSecKeyRef withPublic:pubSecKeyRef];
        [[ToolLogFile defaultLogger] debug:@"SecKeyCopyKeyExchangeResult done"];
        [[ToolLogFile defaultLogger] hexdump:exchangedKey1];
    }

    - (void)testECDH {
        // EC鍵ペアを生成
        id privateSecKeyRef1 = [ToolSecurity generatePrivkeyFromRandom];
        if (privateSecKeyRef1 == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"SecKeyCreateRandomKey done: %@", privateSecKeyRef1];
        // こちらのバイト配列を抽出
        uint8_t pubkeyBytesForTest1[64];
        [ToolSecurity getKeyFromPrivateSecKeyRef:privateSecKeyRef1 toPrivkeyBuffer:NULL toPubkeyBuffer:pubkeyBytesForTest1];
        // 抽出バイト配列から公開鍵をリストア
        id restoredPubkey1 = [ToolSecurity generatePublicSecKeyRefFromPubkeyBytes:pubkeyBytesForTest1];
        if (restoredPubkey1 == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"generatePublicSecKeyRefFromPubkeyBytes(1) done: %@", restoredPubkey1];

        // EC鍵ペアをもう１セット生成（ECDH共通鍵生成用）
        id privateSecKeyRef2 = [ToolSecurity generatePrivkeyFromRandom];
        if (privateSecKeyRef2 == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"SecKeyCreateRandomKey(2) done: %@", privateSecKeyRef2];
        // こちらのバイト配列を抽出
        uint8_t pubkeyBytesForTest2[64];
        [ToolSecurity getKeyFromPrivateSecKeyRef:privateSecKeyRef2 toPrivkeyBuffer:NULL toPubkeyBuffer:pubkeyBytesForTest2];
        // 抽出バイト配列から公開鍵をリストア
        id restoredPubkey2 = [ToolSecurity generatePublicSecKeyRefFromPubkeyBytes:pubkeyBytesForTest2];
        if (restoredPubkey2 == nil) {
            return;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"generatePublicSecKeyRefFromPubkeyBytes(2) done: %@", restoredPubkey2];

        // 共通鍵を生成（１）
        NSData *exchangedKey1 = [ToolSecurity generateECDHSharedSecretWithPrivate:privateSecKeyRef1 withPublic:restoredPubkey2];
        [[ToolLogFile defaultLogger] debugWithFormat:@"SecKeyCopyKeyExchangeResult(1) done: %@", exchangedKey1];
        // 共通鍵を生成（２）
        NSData *exchangedKey2 = [ToolSecurity generateECDHSharedSecretWithPrivate:privateSecKeyRef2 withPublic:restoredPubkey1];
        [[ToolLogFile defaultLogger] debugWithFormat:@"SecKeyCopyKeyExchangeResult(2) done: %@", exchangedKey2];
    }

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

@end
