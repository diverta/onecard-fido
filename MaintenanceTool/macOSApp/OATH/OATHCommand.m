//
//  OATHCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "AppCommonMessage.h"
#import "OATHCommand.h"
#import "QRCodeUtil.h"
#import "ToolCCIDHelper.h"

// コマンドクラスのインスタンスを保持
static OATHCommand *sharedInstance;

@implementation OATHCommandParameter

@end

@interface OATHCommand () <ToolCCIDHelperDelegate>

    // ヘルパークラスの参照を保持
    @property (nonatomic) ToolCCIDHelper               *toolCCIDHelper;

@end

@implementation OATHCommand

#pragma mark - Methods for singleton

    + (OATHCommand *)instance {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] init];
        });
        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Methods of this instance

    - (id)init {
        self = [super init];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
            [self setParameter:[[OATHCommandParameter alloc] init]];
        }
        return self;
    }

    - (bool)isUSBCCIDCanConnect {
        // USB CCIDインターフェースに接続可能でない場合は false
        return [[self toolCCIDHelper] checkHelperCanConnect];
    }

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw {
    }

    - (bool)ScanQRCode {
        // QRコードのスキャンを実行
        NSString *message = [QRCodeUtil scanQRCodeFromScreenShot];
        if (message == nil) {
            [[self parameter] setResultInformativeMessage:MSG_ERROR_OATH_QRCODE_SCAN_FAILED];
            return false;
        }
        QRCodeUtil *qrCodeUtil = [[QRCodeUtil alloc] initWithQRMessageString:message];
        return true;
    }

@end
