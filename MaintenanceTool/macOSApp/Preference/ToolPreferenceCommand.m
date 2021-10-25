//
//  ToolPreferenceCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/24.
//
#import <Foundation/Foundation.h>
#import "ToolPreferenceCommand.h"
#import "ToolPreferenceWindow.h"
#import "ToolAppCommand.h"
#import "ToolHIDCommand.h"

@interface ToolPreferenceCommand ()

    @property (nonatomic, weak) ToolAppCommand      *toolAppCommand;
    @property (nonatomic, weak) ToolHIDCommand      *toolHIDCommand;
    @property (nonatomic) ToolPreferenceWindow      *toolPreferenceWindow;
    @property (nonatomic) ToolPreferenceCommandType commandType;

    // コマンドを実行するためのリクエストデータを保持
    @property (nonatomic) NSData *processData;

@end

@implementation ToolPreferenceCommand

    - (id)init {
        return [self initWithDelegate:nil toolHIDCommandRef:nil];
    }

    - (id)initWithDelegate:(id)delegate toolHIDCommandRef:(id)ref {
        self = [super init];
        if (self) {
            [self setToolAppCommand:(ToolAppCommand *)delegate];
            [self setToolHIDCommand:(ToolHIDCommand *)ref];
        }

        // 使用するダイアログを生成
        [self setToolPreferenceWindow:[[ToolPreferenceWindow alloc]
                                     initWithWindowNibName:@"ToolPreferenceWindow"]];
        return self;
    }

#pragma mark - Main process

    - (void)generateRequestCommandAuthParamGet:(ToolPreferenceCommandType)commandType {
        // 1バイト目＝コマンドバイト
        char cmd[] = {(char)commandType};
        NSMutableData *requestData = [[NSMutableData alloc] initWithBytes:cmd length:1];
        // ２バイト目以降＝設定するデータ（有効化、ペアリング要否、UUID、スキャン秒数）
        uint16_t pairNeed = (uint16_t)[self blePairingIsNeeded] * 256;
        uint16_t flags = (uint16_t)[self bleScanAuthEnabled] + pairNeed;
        NSString *csv = [NSString stringWithFormat:@"%d,%@,%@",
                         flags, [self serviceUUIDString], [self serviceUUIDScanSec]
                         ];
        NSData *data = [csv dataUsingEncoding:NSUTF8StringEncoding];
        // リクエストデータを編集し、内部変数に設定
        if (commandType == COMMAND_AUTH_PARAM_SET) {
            [requestData appendData:data];
        }
        [self setProcessData:requestData];
    }

    - (bool)parseResponseCommandAuthParamGet:(ToolPreferenceCommandType)commandType
                                    fromData:(NSData *)data {
        if (data == nil || [data length] < 1) {
            // 例外抑止
            [self setBleScanAuthEnabled:false];
            [self setServiceUUIDString:@""];
            [self setServiceUUIDScanSec:@""];
            return false;
        }
        // ステータスバイト＝レスポンスの1バイト目
        NSData *statusByte  = [data subdataWithRange:NSMakeRange(0, 1)];
        uint8_t *status = (uint8_t *)[statusByte bytes];
        if (status[0] != 0x00) {
            // ステータスバイトが0x00以外であればエラー
            return false;
        }
        // CSV＝レスポンスの2バイト目以降
        NSData *csvData  = [data subdataWithRange:NSMakeRange(1, [data length] - 1)];
        NSString *csvString = [[NSString alloc] initWithData:csvData
                                                    encoding:NSUTF8StringEncoding];
        // カンマを境にしてCSVを分解
        NSArray<NSString *> *items = [csvString componentsSeparatedByString:@","];
        // 画面項目に設定
        int flags = [[items objectAtIndex:0] intValue];
        int _BleScanAuthEnabled = flags % 256;
        int _BlePairingIsNeeded = flags / 256;
        [self setBleScanAuthEnabled:(_BleScanAuthEnabled == 1)];
        [self setBlePairingIsNeeded:(_BlePairingIsNeeded == 1)];
        [self setServiceUUIDString:[items objectAtIndex:1]];
        [self setServiceUUIDScanSec:[items objectAtIndex:2]];
        return true;
    }

#pragma mark - Interface for AppDelegate

    - (void)toolPreferenceWillProcess:(ToolPreferenceCommandType)commandType {
        // USBポートに装着されているかどうかチェック
        if (![[self toolAppCommand] checkForHIDCommand]) {
            return;
        }

        // 開始ログを出力
        [[self toolPreferenceWindow] toolPreferenceCommandDidStart];
        
        // リクエストデータを編集し、内部変数に設定
        [self setCommandType:commandType];
        [self generateRequestCommandAuthParamGet:[self commandType]];

        // HID経由でコマンドを実行
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TOOL_PREF_PARAM withData:[self processData] forCommand:self];
    }

    - (void)toolPreferenceInquiryWillProcess {
        // リクエストデータを編集し、内部変数に設定
        [self setCommandType:COMMAND_AUTH_PARAM_GET];
        [self generateRequestCommandAuthParamGet:[self commandType]];

        // HID経由でコマンドを実行
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_TOOL_PREF_PARAM_INQUIRY withData:[self processData] forCommand:self];
    }

    - (void)hidCommandDidProcess:(Command)command CMD:(uint8_t)cmd response:(NSData *)resp {
        // 取得データをクラス変数に設定
        bool success = [self parseResponseCommandAuthParamGet:[self commandType] fromData:resp];
        if (command == COMMAND_TOOL_PREF_PARAM) {
            // 画面に制御を戻す
            [[self toolPreferenceWindow] toolPreferenceCommandDidProcess:[self commandType]
                                                                 success:success message:nil];
        } else {
            // 上位コマンドクラスに再び制御を戻す
            [[self toolAppCommand] toolPreferenceInquiryDidProcess:success];
        }
    }

#pragma mark - Interface for ToolPreferenceWindow

    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // すでにツール設定画面が開いている場合は終了
        if ([[parentWindow sheets] count] > 0) {
            return;
        }
        // ダイアログの親ウィンドウを保持
        [[self toolPreferenceWindow] setParentWindow:parentWindow];
        [[self toolPreferenceWindow] setToolPreferenceCommand:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self toolPreferenceWindow] window];
        ToolPreferenceCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolPreferenceWindowDidClose:sender modalResponse:response];
        }];
    }

    - (void)toolPreferenceWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self toolPreferenceWindow] close];
        // ホーム画面に制御を戻す（ポップアップメッセージは表示しない）
        [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
    }

@end
