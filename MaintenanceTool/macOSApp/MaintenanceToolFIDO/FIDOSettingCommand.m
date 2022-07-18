//
//  FIDOSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/18.
//
#import "FIDOSettingCommand.h"

@implementation FIDOSettingCommandParameter

@end

@interface FIDOSettingCommand ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // PIN番号管理処理のパラメーターを保持
    @property (nonatomic) FIDOSettingCommandParameter  *commandParameter;

@end

@implementation FIDOSettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[FIDOSettingCommandParameter alloc] init]];
        }
        return self;
    }

    - (void)fidoSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
    }

    - (bool)isUSBHIDConnected {
        // TODO: USBポートに接続されていない場合はfalse
        return false;
    }

@end
