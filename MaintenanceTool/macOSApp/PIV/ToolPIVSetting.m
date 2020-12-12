//
//  ToolPIVSetting.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/12.
//
#import "ToolPIVCommon.h"
#import "ToolPIVSetting.h"

#define TEMPLATE_PIV_SLOT_NAME              @"Device: %@\n"
#define TEMPLATE_PIV_CHUID                  @"CHUID:  %@\n"
#define TEMPLATE_PIV_CCC                    @"CCC:    %@\n"
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
        // リトライカウンター
        [description appendFormat:TEMPLATE_PIV_PIN_RETRIES, [self retries]];
        return [description copy];
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
