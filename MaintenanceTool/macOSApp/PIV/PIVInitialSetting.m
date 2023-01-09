//
//  PIVInitialSetting.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/09.
//
#import "tool_piv_admin.h"
#import "PIVInitialSetting.h"

@interface PIVInitialSetting ()

    // インポート処理で必要となるAPDUを保持
    @property (nonatomic) NSData           *chuidAPDU;
    @property (nonatomic) NSData           *cccAPDU;

@end

@implementation PIVInitialSetting

#pragma mark - Public methods

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

@end
