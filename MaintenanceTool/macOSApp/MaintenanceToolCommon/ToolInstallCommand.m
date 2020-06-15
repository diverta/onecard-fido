//
//  ToolInstallCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#import <Foundation/Foundation.h>

#import "ToolCommonMessage.h"
#import "ToolInstallCommand.h"
#import "ToolLogFile.h"

// for extracting KeyAgreement
#include "CBORDecoder.h"
#include "CBOREncoder.h"
#include "FIDODefines.h"

// for validate key and cert
#include "fido_crypto.h"

// for debug
#define LOG_HEXDUMP_KEY_AGREEMENT false

@interface ToolInstallCommand ()

@end

@implementation ToolInstallCommand

    - (id)init {
        self = [super init];
        return self;
    }

#pragma mark - Command functions

    - (NSData *)generateEraseSkeyCertMessage:(Command)command {
        return [[NSData alloc] init];
    }

    - (bool)extractKeyAgreement:(NSData *)keyAgreementResponse {
        // GetKeyAgreementレスポンスから公開鍵を抽出
        uint8_t *keyAgreement = (uint8_t *)[keyAgreementResponse bytes];
        size_t   keyAgreementSize = [keyAgreementResponse length];
        uint8_t  status_code = ctap2_cbor_decode_get_agreement_key(keyAgreement, keyAgreementSize);
        if (status_code != CTAP1_ERR_SUCCESS) {
            [self setLastErrorMessage:MSG_CANNOT_RECV_DEVICE_PUBLIC_KEY];
            return false;
        }

#if LOG_HEXDUMP_KEY_AGREEMENT
        [[ToolLogFile defaultLogger] debugWithFormat:@"pubkey_X %@",
         [[NSData alloc] initWithBytes:ctap2_cbor_decode_agreement_pubkey_X() length:32]];
        [[ToolLogFile defaultLogger] debugWithFormat:@"pubkey_Y %@",
         [[NSData alloc] initWithBytes:ctap2_cbor_decode_agreement_pubkey_Y() length:32]];
#endif

        return true;
    }

    - (NSData *)generateSkeyCertInstallCbor:(NSData *)skeyCertBinaryData {
        uint8_t *skeyCertBytes = (uint8_t *)[skeyCertBinaryData bytes];
        size_t skeyCertBytesLength = [skeyCertBinaryData length];

        uint8_t status_code = maintenance_cbor_encode_install_skey_cert(
                                    ctap2_cbor_decode_agreement_pubkey_X(),
                                    ctap2_cbor_decode_agreement_pubkey_Y(),
                                    skeyCertBytes, skeyCertBytesLength);

        if (status_code == CTAP1_ERR_SUCCESS) {
            return [[NSData alloc] initWithBytes:ctap2_cbor_encode_request_bytes()
                                          length:ctap2_cbor_encode_request_bytes_size()];
        } else {
            [self setLastErrorMessage:MSG_CANNOT_CRYPTO_SKEY_CERT_DATA];
            return nil;
        }
        return nil;
    }

    - (NSData *)extractSkeyCertBinaryData:(Command)command
                                  skeyFilePath:(NSString *)skeyFilePath
                                  certFilePath:(NSString *)certFilePath {
        NSMutableData *message = [NSMutableData alloc];

        // 鍵ファイルから秘密鍵（32バイト）を取得し、レスポンスメッセージ領域に格納
        NSData *dataSkey = [self readSkeyFromFile:skeyFilePath];
        if (dataSkey == nil) {
            return nil;
        }

        // 証明書ファイルから内容を取得
        NSData *dataCert = [self readCertFromFile:certFilePath];
        if (dataCert == nil) {
            return nil;
        }

        // 秘密鍵と証明書の整合性検証を行う
        if (validate_skey_cert((uint8_t *)[dataSkey bytes], [dataSkey length],
                               (uint8_t *)[dataCert bytes], [dataCert length]) != CTAP1_ERR_SUCCESS) {
            [self setLastErrorMessage:MSG_INVALID_SKEY_OR_CERT];
            return nil;
        }

        // 秘密鍵を証明書をマージ
        [message appendData:dataSkey];
        [message appendData:dataCert];
        return message;
    }

#pragma mark - Read private key data

    - (NSData *)readSkeyFromFile:(NSString *)skeyFilePath {
        // 鍵ファイルから読み込み
        NSError  *err;
        NSString *str = [NSString
                         stringWithContentsOfFile:skeyFilePath
                         encoding:NSUTF8StringEncoding
                         error:&err];
        if (err.code) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"Secure key file read error: %@", [err description]];
            [self setLastErrorMessage:MSG_CANNOT_READ_SKEY_PEM_FILE];
            return nil;
        }
        
        // 読み込んだデータからヘッダー、フッターを除去
        NSArray *lines = [str componentsSeparatedByString:@"\n"];
        NSMutableString *pem = [NSMutableString string];
        bool headerFound = false;
        for (NSString *line in lines) {
            if ([line length] == 0) {
                continue;
            }
            if ([line compare:@"-----BEGIN EC PRIVATE KEY-----"] == NSOrderedSame) {
                headerFound = true;
                continue;
            }
            if ([line compare:@"-----END EC PRIVATE KEY-----"] == NSOrderedSame) {
                continue;
            }
            if (headerFound == true) {
                [pem appendString:line];
            }
        }
        
        // ヘッダーが見つからない場合はエラー
        if (headerFound == false) {
            [[ToolLogFile defaultLogger] error:@"Secure key file has no header 'BEGIN EC PRIVATE KEY'"];
            [self setLastErrorMessage:MSG_INVALID_SKEY_CONTENT_IN_PEM];
            return nil;
        }
        
        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
        // 先頭からビッグエンディアン形式で配置
        return [self convertSkeyPem:pem];
    }

    - (NSData *)convertSkeyPem:(NSString *)skeyPemContents {
        // Base64エンコードされた秘密鍵文字列をデコード
        NSData *decodedPemData = [[NSData alloc]
                                  initWithBase64EncodedString:skeyPemContents
                                  options:NSDataBase64DecodingIgnoreUnknownCharacters];
        
        // デコードされたデータが39バイト未満の場合はエラー
        if ([decodedPemData length] < 39) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"Secure key has invalid length: %ld", [decodedPemData length]];
            [self setLastErrorMessage:MSG_INVALID_SKEY_LENGTH_IN_PEM];
            return nil;
        }
        
        // 秘密鍵データの先頭6バイト目に「0x0420」というヘッダーがない場合はエラー
        const char *decodedPem = [decodedPemData bytes];
        if (!(decodedPem[5] == 0x04 && decodedPem[6] == 0x20)) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"Secure key has invalid header: 0x%02x%02x", decodedPem[5], decodedPem[6]];
            [self setLastErrorMessage:MSG_INVALID_SKEY_HEADER_IN_PEM];
            return nil;
        }
        
        // 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
        // 先頭からビッグエンディアン形式で配置
        char keyBuffer[32];
        for (int i = 0; i < 32; i++) {
            keyBuffer[i] = decodedPem[7 + i];
        }
        
        // NSDataに変換された秘密鍵を戻す
        NSData *skeyData = [[NSData alloc] initWithBytes:keyBuffer length:sizeof(keyBuffer)];
        return skeyData;
    }

#pragma mark - Read certificate data

    - (NSData *)readCertFromFile:(NSString *)certFilePath {
        // 証明書ファイルから読み込み
        NSData *data = [NSData dataWithContentsOfFile:certFilePath];
        if (data == nil || [data length] == 0) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Cannot read cert file: %@", certFilePath];
            [self setLastErrorMessage:MSG_CANNOT_READ_CERT_CRT_FILE];
            return nil;
        }
        
        // 証明書ファイルの長さが68バイト未満の場合はエラー
        NSUInteger dataCertLength = [data length];
        if (dataCertLength < 68) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"Invalid cert length: %@", dataCertLength];
            [self setLastErrorMessage:MSG_INVALID_CERT_LENGTH_IN_CRT];
            return nil;
        }
        
        return data;
    }

@end
