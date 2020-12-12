//
//  ToolPIVSetting.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/12.
//
#import "ToolPIVSetting.h"

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

@end
