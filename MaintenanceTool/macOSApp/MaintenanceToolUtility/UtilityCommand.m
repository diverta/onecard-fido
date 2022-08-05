//
//  UtilityCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolVersionWindow.h"
#import "UtilityCommand.h"
#import "UtilityWindow.h"

@interface UtilityCommand () <AppHIDCommandDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面の参照を保持
    @property (nonatomic) UtilityWindow                *utilityWindow;
    @property (nonatomic) ToolVersionWindow            *toolVersionWindow;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;

@end

@implementation UtilityCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // 画面のインスタンスを生成
            [self setUtilityWindow:[[UtilityWindow alloc] initWithWindowNibName:@"UtilityWindow"]];
            [self setToolVersionWindow:[[ToolVersionWindow alloc] initWithWindowNibName:@"ToolVersionWindow"]];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            // バージョン情報をセット
            NSString *version = [NSString stringWithFormat:MSG_FORMAT_APP_VERSION, [ToolCommonFunc getAppVersionString]];
            [[self toolVersionWindow] setVersionInfoWithToolName:MSG_APP_NAME toolVersion:version toolCopyright:MSG_APP_COPYRIGHT];
        }
        return self;
    }

    - (void)utilityWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 画面に親画面参照をセット
        [[self utilityWindow] setParentWindowRef:parentWindow];
        [[self utilityWindow] setCommandRef:self];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self utilityWindow] window];
        UtilityCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf utilityWindowDidClose:self modalResponse:response];
        }];
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Perform functions

    - (void)utilityWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self utilityWindow] close];
        // 実行コマンドにより処理分岐
        switch ([[self utilityWindow] commandToPerform]) {
            case COMMAND_HID_GET_FLASH_STAT:
                // Flash ROM情報取得
                [self doRequestHIDGetFlashStat];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                // バージョン情報取得
                [self doRequestHIDGetVersionInfo];
                break;
            case COMMAND_VIEW_APP_VERSION:
                // バージョン情報画面を表示
                [self toolVersionWindowWillOpen:self parentWindow:[self parentWindow]];
                break;
            case COMMAND_VIEW_LOG_FILE:
                // ログファイル格納フォルダーを表示
                [self viewLogFileFolder];
                break;
            default:
                // メイン画面に制御を戻す
                break;
        }
    }

    - (void)viewLogFileFolder {
        // ログファイル格納フォルダーをFinderで表示
        NSURL *url = [NSURL fileURLWithPath:[[ToolLogFile defaultLogger] logFilePathString] isDirectory:false];
        NSArray *fileURLs = [NSArray arrayWithObjects:url, nil];
        [[NSWorkspace sharedWorkspace] activateFileViewerSelectingURLs:fileURLs];
    }

#pragma mark - HID getting info functions

    - (void)doRequestHIDGetFlashStat {
        // Flash ROM情報取得
        [self setCommand:COMMAND_HID_GET_FLASH_STAT];
        [self setCommandName:PROCESS_NAME_GET_FLASH_STAT];
        // コマンド開始メッセージを画面表示
        [self notifyCommandStarted:[self commandName]];
        // HID経由でFlash ROM情報を取得（コマンド 0xC2 を実行、メッセージ無し）
        [[self appHIDCommand] doRequestCommand:[self command] withCMD:HID_CMD_GET_FLASH_STAT withData:nil];
    }

    - (void)doResponseHIDGetFlashStat:(NSData *)message {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        [[ToolLogFile defaultLogger] debugWithFormat:@"Flash ROM statistics: %@", responseCSV];
        // 情報取得CSVから空き領域に関する情報を抽出
        NSString *strUsed = @"";
        NSString *strAvail = @"";
        NSString *strCorrupt = @"";
        for (NSString *element in [responseCSV componentsSeparatedByString:@","]) {
            NSArray *items = [element componentsSeparatedByString:@"="];
            NSString *key = [items objectAtIndex:0];
            NSString *val = [items objectAtIndex:1];
            if ([key isEqualToString:@"words_used"]) {
                strUsed = val;
            } else if ([key isEqualToString:@"words_available"]) {
                strAvail = val;
            } else if ([key isEqualToString:@"corruption"]) {
                strCorrupt = val;
            }
        }
        // 空き容量、破損状況を画面に表示
        NSString *rateText;
        if ([strUsed length] > 0 && [strAvail length] > 0) {
            float avail = [strAvail floatValue];
            float remaining = avail - [strUsed floatValue];
            float rate = remaining / avail * 100.0;
            rateText = [NSString stringWithFormat:MSG_FSTAT_REMAINING_RATE, rate];
        } else {
            rateText = MSG_FSTAT_NON_REMAINING_RATE;
        }
        NSString *corruptText = [strCorrupt isEqualToString:@"0"] ?
            MSG_FSTAT_CORRUPTING_AREA_NOT_EXIST : MSG_FSTAT_CORRUPTING_AREA_EXIST;
        // 画面に制御を戻す
        NSString *displayMessage = [NSString stringWithFormat:@"  %1$@%2$@", rateText, corruptText];
        [[self delegate] notifyMessageToMainUI:displayMessage];
        [self notifyCommandTerminated:[self commandName] message:nil success:true fromWindow:[self parentWindow]];
    }

    - (void)doRequestHIDGetVersionInfo {
        // バージョン情報取得
        [self setCommand:COMMAND_HID_GET_VERSION_INFO];
        [self setCommandName:PROCESS_NAME_GET_VERSION_INFO];
        // コマンド開始メッセージを画面表示
        [self notifyCommandStarted:[self commandName]];
        // HID経由でFlash ROM情報を取得（コマンド 0xC3 を実行、メッセージ無し）
        [[self appHIDCommand] doRequestCommand:[self command] withCMD:HID_CMD_GET_VERSION_INFO withData:nil];
    }

    - (void)doResponseHIDGetVersionInfo:(NSData *)message {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:message];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        NSString *strDeviceName = array[0];
        NSString *strFWRev = array[1];
        NSString *strHWRev = array[2];
        NSString *strSecic = array[3];
        // 画面に制御を戻す
        [[self delegate] notifyMessageToMainUI:MSG_VERSION_INFO_HEADER];
        [[self delegate] notifyMessageToMainUI:[NSString stringWithFormat:MSG_VERSION_INFO_DEVICE_NAME, strDeviceName]];
        [[self delegate] notifyMessageToMainUI:[NSString stringWithFormat:MSG_VERSION_INFO_FW_REV, strFWRev]];
        [[self delegate] notifyMessageToMainUI:[NSString stringWithFormat:MSG_VERSION_INFO_HW_REV, strHWRev]];
        // セキュアICの搭載有無を表示
        if ([strSecic length] > 0) {
            [[self delegate] notifyMessageToMainUI:MSG_VERSION_INFO_SECURE_IC_AVAIL];
        } else {
            [[self delegate] notifyMessageToMainUI:MSG_VERSION_INFO_SECURE_IC_UNAVAIL];
        }
        [self notifyCommandTerminated:[self commandName] message:nil success:true fromWindow:[self parentWindow]];
    }

#pragma mark - Version window

    - (void)toolVersionWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 画面に親画面参照をセット
        [[self toolVersionWindow] setParentWindowRef:parentWindow];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [[self toolVersionWindow] window];
        UtilityCommand * __weak weakSelf = self;
        [parentWindow beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolVersionWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)toolVersionWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self toolVersionWindow] close];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
        // USB接続検知メッセージを表示／出力（このコマンドで代表して実行）
        [[self delegate] notifyMessageToMainUI:MSG_HID_CONNECTED];
        [[ToolLogFile defaultLogger] info:MSG_HID_CONNECTED];
    }

    - (void)didDetectRemoval {
        // USB切断検知メッセージを表示／出力（このコマンドで代表して実行）
        [[self delegate] notifyMessageToMainUI:MSG_HID_REMOVED];
        [[ToolLogFile defaultLogger] info:MSG_HID_REMOVED];
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時でアプリケーションに制御を戻す
        if (success == false) {
            [self notifyCommandTerminated:[self commandName] message:errorMessage success:success fromWindow:[self parentWindow]];
            return;
        }
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [self notifyCommandTerminated:[self commandName] message:MSG_OCCUR_UNKNOWN_ERROR success:false fromWindow:[self parentWindow]];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_GET_FLASH_STAT:
                [self doResponseHIDGetFlashStat:response];
                break;
            case COMMAND_HID_GET_VERSION_INFO:
                [self doResponseHIDGetVersionInfo:response];
                break;
            default:
                // メイン画面に制御を戻す
                [self notifyCommandTerminated:[self commandName] message:nil success:success fromWindow:[self parentWindow]];
                break;
        }
    }

@end
