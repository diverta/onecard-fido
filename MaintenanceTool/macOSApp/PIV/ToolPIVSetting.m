//
//  ToolPIVSetting.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/12.
//
#import "tool_crypto_certificate.h"
#import "tool_piv_admin.h"

#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolPIVCommon.h"
#import "ToolPIVSetting.h"
#import "ToolLogFile.h"

#define TEMPLATE_PIV_SLOT_NAME              @"Device: %@\n"
#define TEMPLATE_PIV_CHUID                  @"CHUID:  %@\n"
#define TEMPLATE_PIV_CCC                    @"CCC:    %@\n"
#define TEMPLATE_PIV_CERT_HEADER            @"Slot for %@ (%s, not after %s)\n"
#define TEMPLATE_PIV_CERT_SUBJECT           @"  Subject:  %s\n"
#define TEMPLATE_PIV_CERT_ISSUER            @"  Issuer:   %s\n"
#define TEMPLATE_PIV_CERT_HASH              @"  SHA-256:  %@\n"
#define TEMPLATE_PIV_CERT_NAME_PAUTH        @"PIV authenticate"
#define TEMPLATE_PIV_CERT_NAME_SIGN         @"signature"
#define TEMPLATE_PIV_CERT_NAME_KEYMGM       @"key management"
#define TEMPLATE_PIV_CERT_HEADER_UNAVAIL    @"Slot for %@ (Not available)\n"
#define TEMPLATE_PIV_PIN_RETRIES            @"PIN tries left: %d"

@interface ToolPIVSetting ()

    // CCID I/F接続デバイス名を保持
    @property (nonatomic) NSString          *slotName;
    // PIV PINリトライカウンターを保持
    @property (nonatomic) uint8_t            retries;
    // PIVデータオブジェクト（statusコマンドの実行結果）を保持
    @property (nonatomic) NSMutableDictionary<NSNumber *, NSData *> *objectDictionary;

@end

@implementation ToolPIVSetting

    - (id)initWithSlotName:(NSString *)name {
        self = [super init];
        if (self) {
            // 接続デバイス名を保持
            [self setSlotName:name];
            // 連想配列を初期化
            [self setObjectDictionary:[NSMutableDictionary dictionary]];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)setRetryCount:(uint8_t)retries {
        // リトライカウンターを設定
        [self setRetries:retries];
    }

    - (void)setDataObject:(NSData *)object forObjectId:(unsigned int)objectId {
        // PIVデータオブジェクトを設定
        [[self objectDictionary] setObject:object forKey:[NSNumber numberWithUnsignedInt:objectId]];
    }

    - (NSString *)getDescriptionString {
        NSMutableString *description = [[NSMutableString alloc] init];
        // デバイス名
        [description appendFormat:TEMPLATE_PIV_SLOT_NAME, [self slotName]];
        // CHUID
        [description appendFormat:TEMPLATE_PIV_CHUID, [self printableCHUIDString]];
        // CCC
        [description appendFormat:TEMPLATE_PIV_CCC, [self printableCCCString]];
        // 証明書
        [self appendCertDescriptionTo:description withObjectId:@PIV_OBJ_AUTHENTICATION withObjectName:TEMPLATE_PIV_CERT_NAME_PAUTH];
        [self appendCertDescriptionTo:description withObjectId:@PIV_OBJ_SIGNATURE withObjectName:TEMPLATE_PIV_CERT_NAME_SIGN];
        [self appendCertDescriptionTo:description withObjectId:@PIV_OBJ_KEY_MANAGEMENT withObjectName:TEMPLATE_PIV_CERT_NAME_KEYMGM];
        // リトライカウンター
        [description appendFormat:TEMPLATE_PIV_PIN_RETRIES, [self retries]];
        return [description copy];
    }

    - (void)appendCertDescriptionTo:(NSMutableString *)description withObjectId:(NSNumber *)objectId withObjectName:(NSString *)objectName {
        // 指定IDのPIVデータオブジェクトを抽出
        if ([self extractCertDescriptionWithObjectId:objectId]) {
            // データ種類名、アルゴリズム、有効期限、発行先／元、SHA-256ハッシュを表示
            CERT_DESC *cert_desc = tool_crypto_certificate_extracted_descriptions();
            [description appendFormat:TEMPLATE_PIV_CERT_HEADER, objectName, cert_desc->alg_name, cert_desc->not_after];
            [description appendFormat:TEMPLATE_PIV_CERT_SUBJECT, cert_desc->subject];
            [description appendFormat:TEMPLATE_PIV_CERT_ISSUER, cert_desc->issuer];
            [description appendFormat:TEMPLATE_PIV_CERT_HASH, [self printableHashStringOfBytes:cert_desc->hash]];
        } else {
            // データ表示不可
            [description appendFormat:TEMPLATE_PIV_CERT_HEADER_UNAVAIL, objectName];
        }
    }

    - (bool)extractCertDescriptionWithObjectId:(NSNumber *)objectId {
        // 指定IDのPIVデータオブジェクトを抽出
        NSData *data = [[self objectDictionary] objectForKey:objectId];
        if ([data length] == 0) {
            return false;
        }
        // PIVデータオブジェクトのデータ本体をTLVから抽出
        if (tool_piv_admin_extract_cert_from_TLV((unsigned char *)[data bytes], [data length]) == false) {
            return false;
        }
        // 必要な属性をPIVデータオブジェクトから抽出
        if (tool_crypto_certificate_extract_descriptions(tool_piv_admin_extracted_cert_data(), tool_piv_admin_extracted_cert_size()) == false) {
            [ToolCommon logErrorMessageWithFuncError:MSG_ERROR_PIV_CERT_INFO_GET_FAILED];
            return false;
        }
        return true;
    }

    - (NSString *)printableHashStringOfBytes:(uint8_t *)bytes {
        // ハッシュを表示可能文字列に変換
        NSData *data = [[NSData alloc] initWithBytes:bytes length:CERT_HASH_MAX_SIZE];
        return [self hexStringWithData:data];
    }

    - (NSString *)printableCHUIDString {
        // CHUIDを表示可能文字列に変換
        NSData *data = [[self objectDictionary] objectForKey:@PIV_OBJ_CHUID];
        return [self printableObjectStringWithData:data];
    }

    - (NSString *)printableCCCString {
        // CCCを表示可能文字列に変換
        NSData *data = [[self objectDictionary] objectForKey:@PIV_OBJ_CAPABILITY];
        return [self printableObjectStringWithData:data];
    }

    - (NSString *)printableObjectStringWithData:(NSData *)data {
        // ブランクデータの場合
        if ([data length] == 0) {
            return @"No data available";
        }
        // オブジェクトの先頭２バイト（＝TLVタグ）は不要なので削除
        uint8_t offset = 2;
        NSUInteger size = [data length] - offset;
        NSData *subdata = [data subdataWithRange:NSMakeRange(offset, size)];
        // データオブジェクトを、表示可能なHEX文字列に変換
        return [self hexStringWithData:subdata];
    }

    - (NSString *)hexStringWithData:(NSData *)data {
        // バイナリーデータを、表示可能なHEX文字列に変換
        uint8_t *bytes = (uint8_t *)[data bytes];
        NSMutableString *hex = [[NSMutableString alloc] init];
        for (int i = 0; i < [data length]; i++) {
            [hex appendFormat:@"%02x", bytes[i]];
        }
        return [hex copy];
    }

@end
