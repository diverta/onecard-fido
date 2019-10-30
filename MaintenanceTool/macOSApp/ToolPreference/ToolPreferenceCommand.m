//
//  ToolPreferenceCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/10/24.
//
#import <Foundation/Foundation.h>
#import "ToolPreferenceCommand.h"
#import "ToolPreferenceWindow.h"

@interface ToolPreferenceCommand ()

    @property (nonatomic, weak) AppDelegate         *delegate;
    @property (nonatomic) ToolPreferenceWindow      *toolPreferenceWindow;
    @property (nonatomic) ToolPreferenceCommandType commandType;

    // コマンドを実行するためのリクエストデータを保持
    @property (nonatomic) NSData *processData;

@end

@implementation ToolPreferenceCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
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
        // ２バイト目以降＝設定するデータ（UUID、スキャン秒数）
        NSString *csv = [NSString stringWithFormat:@"%@,%@",
                         [self serviceUUIDString], [self serviceUUIDScanSec]];
        NSData *data = [csv dataUsingEncoding:NSUTF8StringEncoding];
        // リクエストデータを編集し、内部変数に設定
        if (commandType == COMMAND_AUTH_PARAM_SET) {
            [requestData appendData:data];
        }
        [self setProcessData:requestData];
    }

    - (void)parseResponseCommandAuthParamGet:(ToolPreferenceCommandType)commandType
                                    fromData:(NSData *)data {
        if (data == nil || [data length] < 1) {
            // 例外抑止
            [self setServiceUUIDString:@""];
            [self setServiceUUIDScanSec:@""];
            return;
        }
        // 仮のコードです。
        data = [self generateDummyData];
        // CSV＝レスポンスの2バイト目以降
        NSData *csvString  = [data subdataWithRange:NSMakeRange(1, [data length] - 1)];
        // カンマを境にしてCSVを分解
        NSData *uuid = [csvString subdataWithRange:NSMakeRange(0, [csvString length] - 2)];
        NSData *sec = [csvString subdataWithRange:NSMakeRange([csvString length] - 1, 1)];
        // 画面項目に設定
        [self setServiceUUIDString:[[NSString alloc] initWithData:uuid
                                                         encoding:NSUTF8StringEncoding]];
        [self setServiceUUIDScanSec:[[NSString alloc] initWithData:sec
                                                          encoding:NSUTF8StringEncoding]];
    }

    - (NSData *)generateDummyData {
        // 仮のコードです。後日削除します。
        char cmd[] = {0x00};
        NSMutableData *requestData = [[NSMutableData alloc] initWithBytes:cmd length:1];
        NSString *csv = @"DEADBEEF-E141-11E5-A837-0800200C9A66,3";
        [requestData appendData:[csv dataUsingEncoding:NSUTF8StringEncoding]];
        NSLog(@"generateDummyData[%@]", requestData);
        return requestData;
    }

#pragma mark - Interface for AppDelegate

    - (void)toolPreferenceWillProcess:(ToolPreferenceCommandType)commandType {
        // USBポートに装着されているかどうかチェック
        if (![[self delegate] checkUSBHIDConnection]) {
            return;
        }

        // リクエストデータを編集し、内部変数に設定
        [self setCommandType:commandType];
        [self generateRequestCommandAuthParamGet:[self commandType]];
        NSLog(@"toolPreferenceWillProcess: commandType[%ld] request[%@]",
              (long)commandType, [self processData]);

        // AppDelegate経由でコマンドを実行
        [[self delegate] toolPreferenceWillProcess:COMMAND_TOOL_PREF_PARAM withData:[self processData]];
    }

    - (void)toolPreferenceDidProcess:(Command)command
                                 CMD:(uint8_t)cmd response:(NSData *)resp
                              result:(bool)result message:(NSString *)message {
        NSLog(@"toolPreferenceDidProcess: cmd[%02x] response[%@]", cmd, resp);

        // 取得データを画面項目に設定し、画面に制御を戻す
        [self parseResponseCommandAuthParamGet:[self commandType] fromData:resp];
        [[self toolPreferenceWindow] toolPreferenceCommandDidProcess:[self commandType]
                                                             success:result message:message];
    }

#pragma mark - Interface for ToolPreferenceWindow

    - (void)toolPreferenceWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
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
        // AppDelegateに制御を戻す（ポップアップメッセージは表示しない）
        AppDelegate *app = (AppDelegate *)[self delegate];
        [app toolPreferenceWindowDidClose];
    }

@end
