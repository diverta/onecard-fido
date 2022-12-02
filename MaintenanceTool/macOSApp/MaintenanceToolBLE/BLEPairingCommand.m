//
//  BLEPairingCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#import "AppBLECommand.h"
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "AppHIDCommand.h"
#import "BLEPairingCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"

@interface BLEPairingCommand () <AppHIDCommandDelegate, AppBLECommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                  delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppBLECommand            *appBLECommand;
    @property (nonatomic) AppHIDCommand            *appHIDCommand;
    // 実行対象コマンドを保持
    @property (nonatomic) Command                   command;
    // 使用トランスポートを保持
    @property (nonatomic) TransportType             transportType;

@end

@implementation BLEPairingCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppBLECommand:[[AppBLECommand alloc] initWithDelegate:self]];
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)doRequestHidEraseBonds {
        // トランスポートをUSB HIDに設定
        [self setTransportType:TRANSPORT_HID];
        // CTAPHID_INITから実行
        [self setCommand:COMMAND_ERASE_BONDS];
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        switch ([self command]) {
            case COMMAND_ERASE_BONDS:
                [self doRequestEraseBonds];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)doRequestEraseBonds {
        // メッセージを編集し、コマンド 0xC6 を実行
        uint8_t cmd = MNT_COMMAND_BASE | 0x80;
        [self doRequestCtap2Command:COMMAND_ERASE_BONDS withCMD:cmd withData:[ToolCommonFunc commandDataForEraseBondingData]];
    }

    - (void)doResponseEraseBonds:(NSData *)message {
        // 上位クラスに制御を戻す
        [self commandDidProcess:[self checkStatusCode:message] message:nil];
    }

    - (void)doResponseBLEPairing:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] doResponseBLEPairing:result message:message];
    }

#pragma mark - BLE Command/subcommand process

    - (void)doRequestBlePairing {
        // トランスポートをBLEに設定
        [self setTransportType:TRANSPORT_BLE];
        // BLEペアリング処理を実行
        [self setCommand:COMMAND_PAIRING];
        // 書き込むコマンド（APDU）を編集
        unsigned char arr[] = {MNT_COMMAND_PAIRING_REQUEST};
        NSData *commandData = [[NSData alloc] initWithBytes:arr length:sizeof(arr)];
        [self doRequestCtap2Command:COMMAND_PAIRING withCMD:BLE_CMD_MSG withData:commandData];
    }

    - (void)doResponseBlePairing:(NSData *)message {
        // 上位クラスに制御を戻す
        [self commandDidProcess:[self checkStatusCode:message] message:nil];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self doResponseBLEPairing:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_ERASE_BONDS:
                [self doResponseEraseBonds:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self doResponseBLEPairing:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

#pragma mark - Call back from AppBLECommand

    - (void)didResponseCommand:(Command)command response:(NSData *)response {
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_PAIRING:
                [self doResponseBlePairing:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、一旦ヘルパークラスに制御を戻す
                [[self appBLECommand] commandDidProcess:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

    - (void)didCompleteCommand:(Command)command success:(bool)success errorMessage:(NSString *)errorMessage {
        // 上位クラスに制御を戻す
        [self doResponseBLEPairing:success message:errorMessage];
    }

#pragma mark - Private functions

    - (void)doRequestCtap2Command:(Command)command withCMD:(uint8_t)cmd withData:(NSData *)data {
        // コマンドリクエストを、BLE／HIDトランスポート経由で実行
        if ([self transportType] == TRANSPORT_BLE) {
            [[self appBLECommand] doRequestCommand:command withCMD:cmd withData:data];
        }
        if ([self transportType] == TRANSPORT_HID) {
            [[self appHIDCommand] doRequestCtap2Command:command withCMD:cmd withData:data];
        }
    }

    - (void)commandDidProcess:(bool)success message:(NSString *)message {
        // コマンド実行完了後の処理
        if ([self transportType] == TRANSPORT_BLE) {
            // 一旦ヘルパークラスに制御を戻し、BLE切断処理を実行
            [[self appBLECommand] commandDidProcess:success message:message];
        }
        if ([self transportType] == TRANSPORT_HID) {
            // 上位クラスに制御を戻す
            [self doResponseBLEPairing:success message:message];
        }
    }

    - (bool)checkStatusCode:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
            return false;
        }
        // レスポンスメッセージの１バイト目（ステータスコード）を確認
        uint8_t *requestBytes = (uint8_t *)[responseMessage bytes];
        switch (requestBytes[0]) {
            case CTAP1_ERR_SUCCESS:
                return true;
            default:
                [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
        return false;
    }

    - (bool)checkStatusWordOfResponse:(NSData *)responseMessage {
        // レスポンスデータが揃っていない場合はNG
        if (responseMessage == nil || [responseMessage length] == 0) {
            [self displayMessage:MSG_OCCUR_UNKNOWN_ERROR];
            return false;
        }
        // ステータスワード(レスポンスの末尾２バイト)を取得
        NSUInteger statusWord = [self getStatusWordFrom:responseMessage];
        // 成功判定は0x9000
        if (statusWord == 0x9000) {
            return true;
        }
        // ステータスワードチェックがNGの場合
        [self displayMessage:MSG_OCCUR_UNKNOWN_BLE_ERROR];
        return false;
    }

    - (void)displayMessage:(NSString *)message {
        // メッセージを画面表示
        [[self delegate] notifyMessage:message];
    }

    - (NSUInteger)getStatusWordFrom:(NSData *)bleResponseData {
        // BLEレスポンスデータから、ステータスワードを取得する
        NSUInteger length = [bleResponseData length];
        NSData *responseStatusWord = [bleResponseData subdataWithRange:NSMakeRange(length-2, 2)];
        unsigned char *statusWordChar = (unsigned char *)[responseStatusWord bytes];
        NSUInteger statusWord = statusWordChar[0] * 256 + statusWordChar[1];
        
        return statusWord;
    }

@end
