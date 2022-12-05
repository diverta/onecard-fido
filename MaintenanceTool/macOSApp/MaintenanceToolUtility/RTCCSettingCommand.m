//
//  RTCCSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/05.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "FIDODefines.h"
#import "RTCCSettingCommand.h"
#import "ToolLogFile.h"

@interface RTCCSettingCommand () <AppHIDCommandDelegate, AppBLECommandDelegate>

    // 上位クラスの参照を保持（nilを許容します）
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    @property (nonatomic) AppBLECommand                *appBLECommand;
    // コマンド種別を保持
    @property (nonatomic) Command                       command;
    // コマンドパラメーターを保持
    @property (nonatomic) TransportType                 transportType;
    // 処理機能名称を保持
    @property (nonatomic) NSString                     *nameOfCommand;
    // コマンドが成功したかどうかを保持
    @property (nonatomic) bool                          commandSuccess;
    // エラーメッセージテキストを保持
    @property (nonatomic) NSString                     *errorMessageOfCommand;
    // タイムスタンプを保持
    @property (nonatomic) NSString                     *toolTimestamp;
    @property (nonatomic) NSString                     *deviceTimestamp;

@end

@implementation RTCCSettingCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Public methods

    - (void)commandWillPerform:(Command)command withTransportType:(TransportType)type {
        // 実行コマンド／パラメーターを保持
        [self setCommand:command];
        [self setTransportType:type];
        [self notifyProcessStarted];
        // コマンドにより分岐
        switch (command) {
            case COMMAND_RTCC_GET_TIMESTAMP:
                [self doRequestGetTimestamp];
                break;
            default:
                // 画面に制御を戻す
                [self notifyProcessTerminated:false];
                break;
        }
    }


#pragma mark - Get timestamp from RTCC

    - (void)doRequestGetTimestamp {
        // あらかじめ領域を初期化
        [self setToolTimestamp:@""];
        [self setDeviceTimestamp:@""];
        if ([self transportType] == TRANSPORT_HID) {
            // HID経由でタイムスタンプを取得
            uint8_t cmd = MNT_COMMAND_BASE | 0x80;
            [[self appHIDCommand] doRequestCommand:COMMAND_RTCC_GET_TIMESTAMP withCMD:cmd withData:[self commandDataForGetTimestamp]];
        }
        if ([self transportType] == TRANSPORT_BLE) {
            // BLE経由でタイムスタンプを取得
            [[self appBLECommand] doRequestCommand:COMMAND_RTCC_GET_TIMESTAMP withCMD:BLE_CMD_MSG withData:[self commandDataForGetTimestamp]];
        }
    }

    - (void)doResponseHIDGetTimestamp:(NSData *)response {
        // レスポンスを画面表示
        [self doResponseGetTimestamp:response];
        // 画面に制御を戻す
        [self notifyProcessTerminated:true];
    }

    - (void)doResponseGetTimestamp:(NSData *)response {
        // タイムスタンプ文字列はレスポンスの２バイト目から19文字
        char timestampString[20];
        size_t lastPos = sizeof(timestampString) - 1;
        memcpy(timestampString, [response bytes] + 1, lastPos);
        timestampString[lastPos] = 0;
        // 認証器のタイムスタンプ文字列を保持
        [self setDeviceTimestamp:[[NSString alloc] initWithUTF8String:timestampString]];
        // 管理ツールのタイムスタンプを取得して保持
        NSDateFormatter *df = [[NSDateFormatter alloc] init];
        [df setLocale:[[NSLocale alloc] initWithLocaleIdentifier:@"ja_JP"]];
        [df setDateFormat:@"yyyy/MM/dd HH:mm:ss"];
        [self setToolTimestamp:[df stringFromDate:[NSDate date]]];
    }

    - (NSData *)commandDataForGetTimestamp {
        // タイムスタンプ取得用のリクエストデータを生成
        unsigned char arr[] = {MNT_COMMAND_GET_TIMESTAMP};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        return commandData;
    }

#pragma mark - Private common methods

    - (void)notifyProcessStarted {
        // コマンド処理結果を初期化
        [self setCommandSuccess:false];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_RTCC_SET_TIMESTAMP:
                [self setNameOfCommand:PROCESS_NAME_RTCC_SET_TIMESTAMP];
                break;
            default:
                [self setNameOfCommand:PROCESS_NAME_RTCC_GET_TIMESTAMP];
                break;
        }
        // コマンド開始メッセージをログファイルに出力
        if ([self nameOfCommand]) {
            NSString *startMsg = [NSString stringWithFormat:MSG_FORMAT_START_MESSAGE, [self nameOfCommand]];
            [[ToolLogFile defaultLogger] info:startMsg];
        }
    }

    - (void)notifyProcessTerminated:(bool)success {
        // コマンド終了メッセージを生成
        if ([self nameOfCommand]) {
            NSString *endMsg = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [self nameOfCommand],
                                    success ? MSG_SUCCESS : MSG_FAILURE];
            if (success == false) {
                // コマンド異常終了メッセージをログ出力
                [[ToolLogFile defaultLogger] error:endMsg];
            } else {
                // コマンド正常終了メッセージをログ出力
                [[ToolLogFile defaultLogger] info:endMsg];
            }
        }
        // 戻り先がある場合は制御を戻す
        if ([self delegate]) {
            NSArray<NSString *> *timestamps = @[[self toolTimestamp], [self deviceTimestamp]];
            [[self delegate] RTCCSettingCommandDidProcess:[self command] commandName:[self nameOfCommand]
                                            withTimestamp:timestamps withResult:success withErrorMessage:[self errorMessageOfCommand]];
        }
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時でアプリケーションに制御を戻す
        if (success == false) {
            [self setErrorMessageOfCommand:errorMessage];
            [self notifyProcessTerminated:false];
            return;
        }
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[response bytes];
        if (requestBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [self setErrorMessageOfCommand:MSG_OCCUR_UNKNOWN_ERROR];
            [self notifyProcessTerminated:false];
            return;
        }
        // 画面に制御を戻す
        [self doResponseHIDGetTimestamp:response];
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        if (responseBytes[0] != CTAP1_ERR_SUCCESS) {
            // エラーの場合は画面に制御を戻す
            [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
            return;
        }
        // レスポンスを画面表示
        [self doResponseGetTimestamp:response];
        // 一旦ヘルパークラスに制御を戻す-->BLE切断後、didCompleteCommand が呼び出される
        [[self appBLECommand] commandDidProcess:true message:nil];
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 画面に制御を戻す
        [self setErrorMessageOfCommand:errorMessage];
        [self notifyProcessTerminated:success];
    }

@end
