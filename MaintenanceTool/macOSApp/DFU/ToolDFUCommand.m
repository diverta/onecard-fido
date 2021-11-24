//
//  ToolDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/22.
//
#import "ToolAppCommand.h"
#import "ToolDFUCommand.h"
#import "ToolDFUWindow.h"

@interface ToolDFUCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand         *toolAppCommand;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) ToolDFUWindow                *toolDFUWindow;

@end

@implementation ToolDFUCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setToolAppCommand:(ToolAppCommand *)delegate];
        }
        // 画面のインスタンスを生成
        [self setToolDFUWindow:[[ToolDFUWindow alloc] initWithWindowNibName:@"ToolDFUWindow"]];
        return self;
    }

    - (void)toolDFUWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self toolDFUWindow] setParentWindowRef:parentWindow];
        [[self toolDFUWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self toolDFUWindow] window];
        ToolDFUCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolDFUWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        return [[self toolAppCommand] checkForHIDCommand];
    }

#pragma mark - Perform DFU

    - (void)toolDFUWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self toolDFUWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self toolDFUWindow] commandToPerform]) {
            case COMMAND_USB_DFU:
                // ファームウェア更新処理を実行するため、DFU開始画面を表示
                [[self toolAppCommand] dfuProcessWillStart:sender parentWindow:[self parentWindow]];
                break;
            case COMMAND_BLE_DFU:
                // ファームウェア更新処理を実行するため、DFU開始画面を表示
                [[self toolAppCommand] bleDfuProcessWillStart:sender parentWindow:[self parentWindow]];
                break;
            default:
                // メイン画面に制御を戻す
                [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
                break;
        }
    }

@end
