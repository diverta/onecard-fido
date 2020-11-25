//
//  ToolCCIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import "ToolCCIDCommand.h"
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface ToolCCIDCommand ()

    // CCIDインターフェース処理の参照を保持
    @property (nonatomic) ToolCCIDHelper    *toolCCIDHelper;
    // コマンドを保持
    @property (nonatomic) Command            command;
    @property (nonatomic) uint8_t            commandIns;
    // コマンドのパラメーターを保持
    @property (nonatomic) NSString          *pinCodeCur;
    @property (nonatomic) NSString          *pinCodeNew;
    // エラーメッセージテキストを保持
    @property (nonatomic) NSString          *lastErrorMessage;

@end

@implementation ToolCCIDCommand

    - (id)init {
        self = [super init];
        if (self) {
            // ToolCCIDHelperのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] init]];
            [self clearCommandParameters];
        }
        return self;
    }

    - (void)clearCommandParameters {
        // コマンドおよびパラメーターを初期化
        [self setCommand:COMMAND_NONE];
        [self setCommandIns:0x00];
        [self setPinCodeCur:nil];
        [self setPinCodeNew:nil];
        [self setLastErrorMessage:nil];
    }

    - (void)ccidHelperWillProcess:(Command)command {
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                // 機能実行に先立ち、PIVアプレットをSELECT
                [self doSelectApplication];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)ccidHelperDidProcess:(bool)success response:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self commandIns]) {
            case PIV_INS_SELECT_APPLICATION:
                [self doResponsePivInsSelectApplication:resp status:sw];
                break;
            case PIV_INS_VERIFY:
                [self doResponsePivInsVerify:resp status:sw];
                break;
            case PIV_INS_CHANGE_REFERENCE:
            case PIV_INS_RESET_RETRY:
                [self doResponsePivInsChangePin:resp status:sw];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

#pragma mark - Public methods

    - (void)ccidHelperWillChangePin:(Command)command withNewPinCode:(NSString *)pinCodeNew withAuthPinCode:(NSString *)pinCodeCur {
        [self setPinCodeNew:pinCodeNew];
        [self setPinCodeCur:pinCodeCur];
        [self ccidHelperWillProcess:command];
    }

#pragma mark - Command functions

    - (void)doSelectApplication {
        [self setCommandIns:PIV_INS_SELECT_APPLICATION];
        [[self toolCCIDHelper] setSendParameters:self ins:[self commandIns] p1:0x04 p2:0x00 data:[self getPivAidData] le:0xff];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession];
    }

    - (void)doResponsePivInsSelectApplication:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [self setLastErrorMessage:MSG_ERROR_PIV_APPLET_SELECT_FAILED];
            [self exitCommandProcess:false];
            return;
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self doPivInsChangePIN:[self command]];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)doTestPivInsVerify:(NSString *)pinCode {
        // TODO: 将来的に鍵・証明書導入機能で使用予定です。
        // コマンドAPDUを生成
        NSData *apdu = nil;
        if (pinCode != nil) {
            apdu = [self getPivPinVerifyData:pinCode];
        }
        // コマンドを実行
        [self setCommandIns:PIV_INS_VERIFY];
        [[self toolCCIDHelper] setSendParameters:self ins:[self commandIns] p1:0x00 p2:PIV_KEY_PIN data:apdu le:0xff];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession];
    }

    - (void)doResponsePivInsVerify:(NSData *)response status:(uint16_t)sw {
        // TODO: 将来的に鍵・証明書導入機能で使用予定です。
        // for research
        [[ToolLogFile defaultLogger] debugWithFormat:@"doResponsePivInsVerify: RESP[%@] SW[0x%04X]", response, sw];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

    - (void)doPivInsChangePIN:(Command)command {
        // INS、P2を設定
        uint8_t ins, p2;
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                ins = PIV_INS_CHANGE_REFERENCE;
                p2 = PIV_KEY_PIN;
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                ins = PIV_INS_CHANGE_REFERENCE;
                p2 = PIV_KEY_PUK;
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                ins = PIV_INS_RESET_RETRY;
                p2 = PIV_KEY_PIN;
                break;
            default:
                [self exitCommandProcess:false];
                return;
        }
        // コマンドAPDUを生成
        NSData *apdu = [self getPivChangePinData:[self pinCodeCur] withPinCodeNew:[self pinCodeNew]];
        // コマンドを実行
        [self setCommandIns:ins];
        [[self toolCCIDHelper] setSendParameters:self ins:[self commandIns] p1:0x00 p2:p2 data:apdu le:0xff];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession];
    }

    - (void)doResponsePivInsChangePin:(NSData *)response status:(uint16_t)sw {
        // ステータスワードをチェックし、エラーの種類を判定
        uint8_t retries = 3;
        bool isPinBlocked = false;
        if ((sw >> 8) == 0x63) {
            // リトライカウンターが戻された場合（入力PIN／PUKが不正時）
            retries = sw & 0xf;
            if (retries < 1) {
                isPinBlocked = true;
            }

        } else if (sw == SW_ERR_AUTH_BLOCKED) {
            // 入力PIN／PUKがすでにブロックされている場合
            isPinBlocked = true;

        } else if (sw != SW_SUCCESS) {
            // 不明なエラーが発生時
            [self setLastErrorMessage:MSG_ERROR_PIV_UNKNOWN];
        }
        // PINブロック or リトライカウンターの状態に応じメッセージを編集
        bool isPinAuth = ([self command] == COMMAND_CCID_PIV_CHANGE_PIN);
        if (isPinBlocked) {
            [self setLastErrorMessage:isPinAuth ? MSG_ERROR_PIV_PIN_LOCKED : MSG_ERROR_PIV_PUK_LOCKED];

        } else if (retries < 3) {
            NSString *name = isPinAuth ? @"PIN" : @"PUK";
            NSString *msg = [[NSString alloc] initWithFormat:MSG_ERROR_PIV_WRONG_PIN, name, name, retries];
            [self setLastErrorMessage:msg];
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                [self exitCommandProcess:(sw == SW_SUCCESS)];
                break;
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

#pragma mark - Exit function

    - (void)exitCommandProcess:(bool)success {
        if (success == false) {
            // 処理失敗時はエラーメッセージをログ出力
            if ([self lastErrorMessage]) {
                [[ToolLogFile defaultLogger] error:[self lastErrorMessage]];
            }
        }
        // TODO: 画面に制御を戻す
        [[ToolLogFile defaultLogger] infoWithFormat:MSG_FORMAT_END_MESSAGE, @"Command", success ? MSG_SUCCESS : MSG_FAILURE];
        // パラメーターを初期化
        [self clearCommandParameters];
    }

#pragma mark - Utility functions

    - (NSData *)getPivAidData {
        static uint8_t piv_aid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
        return [NSData dataWithBytes:piv_aid length:sizeof(piv_aid)];
    }

    - (NSData *)getPivPinVerifyData:(NSString *)pinCode {
        // PINコードを配列にセット
        uint8_t pin_code[8];
        memset(pin_code, 0xff, sizeof(pin_code));
        if (pinCode != nil) {
            uint8_t *c = (uint8_t *)[pinCode UTF8String];
            size_t s = [pinCode length];
            memcpy(pin_code, c, s);
        }
        // NSData形式に変換
        return [NSData dataWithBytes:pin_code length:sizeof(pin_code)];
    }

    - (NSData *)getPivChangePinData:(NSString *)pinCodeCur withPinCodeNew:(NSString *)pinCodeNew {
        // 認証用PINコード、更新用PINコードの順で配列にセット
        uint8_t pin_code[16];
        memset(pin_code, 0xff, sizeof(pin_code));
        if (pinCodeCur != nil) {
            uint8_t *c = (uint8_t *)[pinCodeCur UTF8String];
            size_t s = [pinCodeCur length];
            memcpy(pin_code, c, s);
        }
        if (pinCodeNew != nil) {
            uint8_t *c = (uint8_t *)[pinCodeNew UTF8String];
            size_t s = [pinCodeNew length];
            memcpy(pin_code + 8, c, s);
        }
        // NSData形式に変換（16バイト固定長）
        return [NSData dataWithBytes:pin_code length:sizeof(pin_code)];
    }

@end
