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
    @property (nonatomic) NSString          *pinCode;

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
        [self setPinCode:nil];
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
            default:
                [self exitCommandProcess:false];
                break;
        }
    }

#pragma mark - Public methods

    - (void)ccidHelperWillProcess:(Command)command withPinCode:(NSString *)pinCode {
        [self setPinCode:pinCode];
        [self ccidHelperWillProcess:command];
    }

#pragma mark - Command functions

    - (void)doSelectApplication {
        [self setCommandIns:PIV_INS_SELECT_APPLICATION];
        [[self toolCCIDHelper] setSendParameters:self ins:[self commandIns] p1:0x04 p2:0x00 data:[self getPivAidData] le:0xff];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession];
    }

    - (void)doResponsePivInsSelectApplication:(NSData *)response status:(uint16_t)sw {
        // for research
        [[ToolLogFile defaultLogger] debugWithFormat:@"doResponseSelectApplication: RESP[%@] SW[0x%04X]", response, sw];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                // TODO: 仮の実装です。
                [self exitCommandProcess:(sw == SW_SUCCESS)];
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

#pragma mark - Exit function

    - (void)exitCommandProcess:(bool)success {
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

@end
