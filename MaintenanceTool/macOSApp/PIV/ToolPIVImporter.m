//
//  ToolPIVImporter.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#import "debug_log.h"
#import "tool_crypto_certificate.h"
#import "tool_crypto_private_key.h"
#import "tool_piv_admin.h"

#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPIVImporter.h"

@interface ToolPIVImporter ()

    // 処理対象となるスロットIDを保持
    @property (nonatomic) uint8_t           keySlotId;
    // インポート処理で必要となるAPDUを保持
    @property (nonatomic) NSData           *certificateAPDU;

@end

@implementation ToolPIVImporter

    - (id)initForKeySlot:(uint8_t)keySlotId {
        self = [super init];
        if (self) {
            [self setKeySlotId:keySlotId];
        }
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
        if (tool_piv_admin_load_certificate([self keySlotId], path) == false) {
            [self logErrorMessageWithFuncError:MSG_ERROR_PIV_CERT_PEM_LOAD_FAILED];
            return false;
        }
        // バイナリーイメージから生成されたAPDUを内部で保持
        NSData *apdu = [[NSData alloc] initWithBytes:tool_piv_admin_cert_APDU_data()
                                              length:tool_piv_admin_cert_APDU_size()];
        [self setCertificateAPDU:apdu];
        [[ToolLogFile defaultLogger] info:MSG_PIV_CERT_PEM_LOADED];
        return true;
    }

    - (NSData *)getPrivateKeyAPDUData; {
        return [[NSData alloc] initWithBytes:tool_crypto_private_key_APDU_data()
                                      length:tool_crypto_private_key_APDU_size()];
    }

    - (NSData *)getCertificateAPDUData {
        return [self certificateAPDU];
    }

#pragma mark - Utility functions

    - (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [[ToolLogFile defaultLogger] error:errorMsg];
    }

@end
