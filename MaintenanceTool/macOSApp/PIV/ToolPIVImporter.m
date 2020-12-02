//
//  ToolPIVImporter.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#import "debug_log.h"
#import "tool_crypto_certificate.h"
#import "tool_crypto_private_key.h"

#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPIVImporter.h"

@interface ToolPIVImporter ()

@end

@implementation ToolPIVImporter

    - (id)init {
        self = [super init];
        return self;
    }

#pragma mark - Public methods

    - (bool)readPrivateKeyPemFrom:(NSString *)pemFilePath {
        // PEM形式の秘密鍵ファイルから、バイナリーイメージを抽出
        char *path = (char *)[pemFilePath UTF8String];
        if (tool_crypto_private_key_extract_from_pem(path) == false) {
            [self logErrorMessageWithFuncError:MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED];
            return false;
        }
        [[ToolLogFile defaultLogger] info:MSG_PIV_PKEY_PEM_LOADED];
        return true;
    }

    - (bool)readCertificatePemFrom:(NSString *)pemFilePath {
        // PEM形式の証明書ファイルから、バイナリーイメージを抽出
        char *path = (char *)[pemFilePath UTF8String];
        if (tool_crypto_certificate_extract_from_pem(path) == false) {
            [self logErrorMessageWithFuncError:MSG_ERROR_PIV_CERT_PEM_LOAD_FAILED];
            return false;
        }
        [[ToolLogFile defaultLogger] info:MSG_PIV_CERT_PEM_LOADED];
        return true;
    }

    - (NSData *)getPrivateKeyTLVData {
        return [[NSData alloc] initWithBytes:tool_crypto_private_key_TLV_data()
                                      length:tool_crypto_private_key_TLV_size()];
    }

    - (NSData *)getCertificateTLVData {
        return [[NSData alloc] initWithBytes:tool_crypto_certificate_TLV_data()
                                      length:tool_crypto_certificate_TLV_size()];
    }

#pragma mark - Utility functions

    - (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [[ToolLogFile defaultLogger] error:errorMsg];
    }

@end
