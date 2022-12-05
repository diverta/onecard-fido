//
//  RTCCSettingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/05.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
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
            default:
                // 画面に制御を戻す
                [self notifyProcessTerminated:false];
                break;
        }
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
            // TODO: 仮の実装です。
            NSArray<NSString *> *timestamps = @[@"2022/12/05 13:00:00", @"2022/12/05 13:00:00"];
            [self setErrorMessageOfCommand:@"TODO: 仮の実装です。"];
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
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
    }

@end
