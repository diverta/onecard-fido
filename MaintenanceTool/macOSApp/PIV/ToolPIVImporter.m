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

#import "AppCommonMessage.h"
#import "ToolCommon.h"
#import "ToolPIVCommon.h"
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
            [ToolCommon logErrorMessageWithFuncError:MSG_ERROR_PIV_PKEY_PEM_LOAD_FAILED];
            return false;
        }
        // バイナリーイメージから生成されたAPDUを内部で保持
        NSData *apdu = [[NSData alloc] initWithBytes:tool_piv_admin_generated_APDU_data()
                                              length:tool_piv_admin_generated_APDU_size()];
        [self setPrivateKeyAPDU:apdu];
        [self setKeyAlgorithm:algorithm];
        return true;
    }

    - (bool)readCertificatePemFrom:(NSString *)pemFilePath {
        // PEM形式の証明書ファイルから、バイナリーイメージを抽出
        char *path = (char *)[pemFilePath UTF8String];
        uint8_t algorithm;
        if (tool_piv_admin_load_certificate([self keySlotId], path, &algorithm) == false) {
            [ToolCommon logErrorMessageWithFuncError:MSG_ERROR_PIV_CERT_PEM_LOAD_FAILED];
            return false;
        }
        // バイナリーイメージから生成されたAPDUを内部で保持
        NSData *apdu = [[NSData alloc] initWithBytes:tool_piv_admin_generated_APDU_data()
                                              length:tool_piv_admin_generated_APDU_size()];
        [self setCertificateAPDU:apdu];
        [self setCertAlgorithm:algorithm];
        return true;
    }

    - (NSData *)getPrivateKeyAPDUData; {
        return [self privateKeyAPDU];
    }

    - (NSData *)getCertificateAPDUData {
        return [self certificateAPDU];
    }

    - (NSString *)getKeySlotName {
        switch([self keySlotId]) {
            case PIV_KEY_AUTHENTICATION:
                return MSG_PIV_KEY_SLOT_NAME_1;
            case PIV_KEY_SIGNATURE:
                return MSG_PIV_KEY_SLOT_NAME_2;
            case PIV_KEY_KEYMGM:
                return MSG_PIV_KEY_SLOT_NAME_3;
            default:
                return @"";
        }
    }

    - (NSString *)getKeyAlgorithmName {
        return [self getAlgorithmName:[self keyAlgorithm]];
    }

    - (NSString *)getCertAlgorithmName {
        return [self getAlgorithmName:[self certAlgorithm]];
    }

    - (NSString *)getAlgorithmName:(uint8_t)alg {
        switch(alg) {
            case 0x07:
                return @"RSA2048";
            case 0x11:
                return @"ECCP256";
            default:
                return @"";
        }
    }

@end
