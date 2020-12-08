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

    // インポート処理で必要となるAPDUを保持
    @property (nonatomic) NSData           *privateKeyAPDU;
    @property (nonatomic) NSData           *certificateAPDU;
    @property (nonatomic) NSData           *chuidAPDU;
    @property (nonatomic) NSData           *cccAPDU;

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
        uint8_t algorithm;
        if (tool_piv_admin_load_private_key([self keySlotId], path, &algorithm) == false) {
            [self logErrorMessageWithFuncError:MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED];
            return false;
        }
        // バイナリーイメージから生成されたAPDUを内部で保持
        NSData *apdu = [[NSData alloc] initWithBytes:tool_piv_admin_generated_APDU_data()
                                              length:tool_piv_admin_generated_APDU_size()];
        [self setPrivateKeyAPDU:apdu];
        [self setAlgorithm:algorithm];
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
        NSData *apdu = [[NSData alloc] initWithBytes:tool_piv_admin_generated_APDU_data()
                                              length:tool_piv_admin_generated_APDU_size()];
        [self setCertificateAPDU:apdu];
        [[ToolLogFile defaultLogger] info:MSG_PIV_CERT_PEM_LOADED];
        return true;
    }

    - (NSData *)getPrivateKeyAPDUData; {
        return [self privateKeyAPDU];
    }

    - (NSData *)getCertificateAPDUData {
        return [self certificateAPDU];
    }

    - (void)generateChuidAndCcc {
        // CHUIDインポート用のAPDUを生成
        size_t size;
        uint8_t *chuidBytes = tool_piv_admin_generate_CHUID_APDU(&size);
        NSData *chuidApdu = [[NSData alloc] initWithBytes:chuidBytes length:size];
        [self setChuidAPDU:chuidApdu];

        // CCCインポート用のAPDUを生成
        uint8_t *cccBytes = tool_piv_admin_generate_CCC_APDU(&size);
        NSData *cccApdu = [[NSData alloc] initWithBytes:cccBytes length:size];
        [self setCccAPDU:cccApdu];
    }

    - (NSData *)getChuidAPDUData {
        return [self chuidAPDU];
    }

    - (NSData *)getCccAPDUData {
        return [self cccAPDU];
    }

#pragma mark - Utility functions

    - (void)logErrorMessageWithFuncError:(NSString *)errorMsgTemplate {
        NSString *functionMsg = [[NSString alloc] initWithUTF8String:log_debug_message()];
        NSString *errorMsg = [[NSString alloc] initWithFormat:errorMsgTemplate, functionMsg];
        [[ToolLogFile defaultLogger] error:errorMsg];
    }

@end
