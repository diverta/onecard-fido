//
//  ToolFIDOAttestationCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/17.
//
#import "FIDOAttestationWindow.h"
#import "ToolAppCommand.h"
#import "ToolFIDOAttestationCommand.h"

@interface ToolFIDOAttestationCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand         *toolAppCommand;
    // 画面の参照を保持
    @property (nonatomic) FIDOAttestationWindow        *fidoAttestationWindow;

@end

@implementation ToolFIDOAttestationCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setToolAppCommand:(ToolAppCommand *)delegate];
        }
        // 画面のインスタンスを生成
        [self setFidoAttestationWindow:[[FIDOAttestationWindow alloc] initWithWindowNibName:@"FIDOAttestationWindow"]];
        return self;
    }

    - (void)fidoAttestationWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 画面に親画面参照をセット
        [[self fidoAttestationWindow] setParentWindowRef:parentWindow];
        [[self fidoAttestationWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self fidoAttestationWindow] window];
        ToolFIDOAttestationCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuStartWindowDidClose:[self toolAppCommand] modalResponse:response];
        }];
    }

    - (void)dfuStartWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self fidoAttestationWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self fidoAttestationWindow] commandToPerform]) {
            case COMMAND_INSTALL_SKEY_CERT:
                [[self toolAppCommand] doCommandInstallSkeyCert:[[self fidoAttestationWindow] selectedFilePaths]];
                break;
            case COMMAND_ERASE_SKEY_CERT:
                [[self toolAppCommand] doCommandEraseSkeyCert];
                break;
            default:
                // メイン画面に制御を戻す
                [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
                break;
        }
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolAppCommand] checkForHIDCommand];
    }

@end
