//
//  ToolSecurity.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/30.
//
#import "ToolLogFile.h"
#import "ToolSecurity.h"

@interface ToolSecurity ()

@end

@implementation ToolSecurity

    + (NSData *)generatePrivkeyDataFromPrivkeyBytes:(uint8_t *)privBytes withPubkeyBytes:(uint8_t *)pubBytes {
        // Securityフレームワークで処理できる形式（0x04 || X || Y || K）に変換
        uint8_t pkeyBytes[97];
        pkeyBytes[0] = 0x04;
        memcpy(pkeyBytes + 1, pubBytes, 64);
        memcpy(pkeyBytes + 1 + 64, privBytes, 32);
        // NSDataに変換して戻す
        return [[NSData alloc] initWithBytes:pkeyBytes length:sizeof(pkeyBytes)];
    }

    + (id)generatePrivkeyFromData:(NSData *)privkeyData {
        // Options (SECP256R1, private)
        NSMutableDictionary *options = [NSMutableDictionary dictionary];
        options[(__bridge id)kSecAttrKeyType]  = (__bridge id)kSecAttrKeyTypeECSECPrimeRandom;
        options[(__bridge id)kSecAttrKeyClass] = (__bridge id)kSecAttrKeyClassPrivate;
        // Create SecKeyRef of EC private key
        CFErrorRef error = NULL;
        id ref = CFBridgingRelease(SecKeyCreateWithData((__bridge CFDataRef)privkeyData, (__bridge CFDictionaryRef)options, &error));
        if (error) {
            NSError *err = CFBridgingRelease(error);
            [[ToolLogFile defaultLogger] errorWithFormat:@"SecKeyCreateWithData: %@", err.description];
            return nil;
        }
        if (ref == nil) {
            [[ToolLogFile defaultLogger] error:@"SecKeyCreateWithData fail"];
        }
        return ref;
    }

    + (id)generatePrivkeyFromRandom {
        // EC鍵ペア生成用の属性を設定（SECP256R1、キーストアに保存しない）
        NSDictionary *attrs = @{
            (__bridge id)kSecClass : (__bridge id)kSecClassKey,
            (__bridge id)kSecAttrKeyType : (__bridge id)kSecAttrKeyTypeECSECPrimeRandom,
            (__bridge id)kSecAttrKeySizeInBits : @256,
            (__bridge id)kSecPrivateKeyAttrs : @{
                (__bridge id)kSecAttrIsPermanent : @NO,
            }
        };
        // EC鍵ペアを生成
        CFErrorRef keyCFError = NULL;
        id privateSecKeyRef = CFBridgingRelease(SecKeyCreateRandomKey((__bridge CFDictionaryRef)attrs, &keyCFError));
        if (keyCFError) {
            NSError *err = CFBridgingRelease(keyCFError);
            [[ToolLogFile defaultLogger] errorWithFormat:@"SecKeyCreateRandomKey: %@", err.description];
            return nil;
        }
        if (privateSecKeyRef == nil) {
            [[ToolLogFile defaultLogger] error:@"SecKeyCreateRandomKey fail"];
        }
        return privateSecKeyRef;
    }

    + (id)generatePubkeyFromPrivkey:(id)privSecKeyRef {
        // Create SecKeyRef of EC public key
        id ref = CFBridgingRelease(SecKeyCopyPublicKey((__bridge SecKeyRef)privSecKeyRef));
        if (ref == nil) {
            [[ToolLogFile defaultLogger] error:@"SecKeyCopyPublicKey fail"];
        }
        return ref;
    }

    + (bool)getKeyFromPrivateSecKeyRef:(id)secKeyRef toPrivkeyBuffer:(uint8_t *)privkeyBytes toPubkeyBuffer:(uint8_t *)pubkeyBytes {
        // Securityフレームワーク内部形式の秘密鍵／公開鍵を、外部表現形式（0x04 || X || Y || K）に変換
        CFErrorRef keyCFError = NULL;
        NSData *keyData = CFBridgingRelease(SecKeyCopyExternalRepresentation((__bridge SecKeyRef)secKeyRef, &keyCFError));
        if (keyData == nil) {
            [[ToolLogFile defaultLogger] error:@"SecKeyCopyExternalRepresentation fail"];
            return false;
        }
        if (keyCFError) {
            NSError *err = CFBridgingRelease(keyCFError);
            [[ToolLogFile defaultLogger] errorWithFormat:@"SecKeyCopyExternalRepresentation: %@", err.description];
            return false;
        }
        // バイト配列を抽出
        if (pubkeyBytes != NULL) {
            [keyData getBytes:pubkeyBytes range:NSMakeRange(1, 64)];
        }
        if (privkeyBytes != NULL) {
            [keyData getBytes:privkeyBytes range:NSMakeRange(65, 32)];
        }
        return true;
    }

    + (NSData *)createECDSASignatureWithData:(NSData *)data withPrivkeyRef:(id)privkey withAlgorithm:(SecKeyAlgorithm)algorithm {
        // 署名アルゴリズムの妥当性チェック
        if (SecKeyIsAlgorithmSupported((__bridge SecKeyRef)privkey, kSecKeyOperationTypeSign, algorithm) == false) {
            [[ToolLogFile defaultLogger] error:@"createECDSASignatureWithData fail: algorithm not supported"];
            return nil;
        }
        // 署名を生成
        CFErrorRef error = NULL;
        NSData *signature = (NSData *)CFBridgingRelease(SecKeyCreateSignature((__bridge SecKeyRef)privkey, algorithm, (__bridge CFDataRef)data, &error));
        if (signature == nil) {
            NSError *err = CFBridgingRelease(error);
            [[ToolLogFile defaultLogger] errorWithFormat:@"createECDSASignatureWithData fail: %@", err.description];
            return nil;
        }
        // 生成された署名を戻す
        return signature;
    }

    + (bool)verifyECDSASignature:(NSData *)signature withDataToSign:(NSData *)dataToSign withPubkeyRef:(id)pubkey withAlgorithm:(SecKeyAlgorithm)algorithm {
        // 署名アルゴリズムの妥当性チェック
        if (SecKeyIsAlgorithmSupported((__bridge SecKeyRef)pubkey, kSecKeyOperationTypeVerify, algorithm) == false) {
            [[ToolLogFile defaultLogger] error:@"verifyECDSASignature fail: algorithm not supported"];
            return false;
        }
        // 署名を検証
        CFErrorRef error = NULL;
        bool verified = SecKeyVerifySignature((__bridge SecKeyRef)pubkey, algorithm, (__bridge CFDataRef)dataToSign, (__bridge CFDataRef)signature, &error);
        if (verified == false) {
            NSError *err = CFBridgingRelease(error);
            [[ToolLogFile defaultLogger] errorWithFormat:@"verifyECDSASignature fail: %@", err.description];
            return false;
        }
        // 署名検証成功
        return true;
    }

@end
