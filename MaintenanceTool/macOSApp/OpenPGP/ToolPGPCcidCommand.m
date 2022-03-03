//
//  ToolPGPCcidCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/03/02.
//
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolCommonMessage.h"
#import "ToolPGPCcidCommand.h"
#import "ToolPGPCommand.h"
#import "ToolPGPCommon.h"

@interface ToolPGPCcidCommand () <ToolCCIDHelperDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id<ToolPGPCcidCommandDelegate> delegate;
    // CCIDインターフェース処理の参照を保持
    @property (nonatomic) ToolCCIDHelper               *toolCCIDHelper;
    @property (nonatomic) uint8_t                       commandIns;
    // コマンド種別を保持
    @property (nonatomic) Command                       command;
    // コマンドパラメーターを保持
    @property (nonatomic) ToolPGPParameter             *commandParameter;

@end

@implementation ToolPGPCcidCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolPGPCcidCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ToolCCIDHelperのインスタンスを生成
            [self setToolCCIDHelper:[[ToolCCIDHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)ccidCommandWillProcess:(Command)command withCommandParameter:(id)parameter {
        // 実行コマンド／パラメーターを保持
        [self setCommand:command];
        [self setCommandParameter:(ToolPGPParameter *)parameter];
        // CCIDインタフェース経由で認証器に接続
        if ([self startCCIDConnection] == false) {
            // 上位クラスに制御を戻す
            [[self delegate] ccidCommandDidProcess:false];
            return;
        }
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            case COMMAND_OPENPGP_INSTALL_KEYS:
                // 機能実行に先立ち、PIVアプレットをSELECT
                [self doRequestOpenPGPInsSelectApplication];
                break;
            default:
                // 上位クラスに制御を戻す
                [self notifyCommandTerminated:false];
                break;
        }
    }

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self commandIns]) {
            case OPENPGP_INS_SELECT:
                [self doResponseOpenPGPInsSelectApplication:resp status:sw];
                break;
            case OPENPGP_INS_VERIFY:
                [self doResponseOpenPGPInsVerify:resp status:sw];
                break;
            default:
                [self notifyCommandTerminated:false];
                break;
        }
    }

#pragma mark - Private methods

    - (bool)startCCIDConnection {
        // CCIDデバイスに接続
        if ([[self toolCCIDHelper] ccidHelperWillConnect]) {
            return true;
        } else {
            // OpenPGP機能を認識できなかった旨のエラーメッセージを設定
            [[self delegate] ccidCommandDidNotifyErrorMessage:MSG_ERROR_OPENPGP_SELECTING_CARD_FAIL];
            return false;
        }
    }

    - (void)notifyCommandTerminated:(bool)success {
        // CCIDデバイスから切断し、上位クラスに制御を戻す
        [[self toolCCIDHelper] ccidHelperWillDisconnect];
        [[self delegate] ccidCommandDidProcess:success];
    }

#pragma mark - Command functions

    - (void)doRequestOpenPGPInsSelectApplication {
        // OpenPGP appletを選択
        static uint8_t aid[] = {0xD2, 0x76, 0x00, 0x01, 0x24, 0x01};
        NSData *aidData = [NSData dataWithBytes:aid length:sizeof(aid)];
        [self setCommandIns:OPENPGP_INS_SELECT];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x04 p2:0x00 data:aidData le:0xff];
    }

    - (void)doResponseOpenPGPInsSelectApplication:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            [[self delegate] ccidCommandDidNotifyErrorMessage:MSG_ERROR_OPENPGP_APPLET_SELECT_FAILED];
            [self notifyCommandTerminated:false];
            return;
        }
        // 次の処理に移行
        [self doRequestOpenPGPInsVerify];
    }

    - (void)doRequestOpenPGPInsVerify {
        // パラメーターの管理用PIN番号を使用し、PIN認証を実行
        [self setCommandIns:OPENPGP_INS_VERIFY];
        NSData *adminPinData = [[[self commandParameter] passphrase] dataUsingEncoding:NSUTF8StringEncoding];
        [[self toolCCIDHelper] ccidHelperWillSendIns:[self commandIns] p1:0x00 p2:0x83 data:adminPinData le:0xff];
    }

    - (void)doResponseOpenPGPInsVerify:(NSData *)response status:(uint16_t)sw {
        // 不明なエラーが発生時は以降の処理を行わない
        if (sw != SW_SUCCESS) {
            // 入力PINが不正の場合はその旨のメッセージを出力
            NSString *errMsg;
            if ((sw & 0xfff0) == SW_PIN_RETRIES) {
                uint16_t retries = sw & 0x000f;
                errMsg = [NSString stringWithFormat:MSG_FORMAT_OPENPGP_PIN_VERIFY_ERR, MSG_LABEL_ITEM_PGP_ADMIN_PIN, retries];
            } else {
                errMsg = [NSString stringWithFormat:MSG_FORMAT_OPENPGP_CARD_EDIT_PASSWD_ERR, MSG_LABEL_COMMAND_OPENPGP_ADMIN_PIN_VERIFY];
            }
            [[self delegate] ccidCommandDidNotifyErrorMessage:errMsg];
            [self notifyCommandTerminated:false];
            return;
        }
        // 上位クラスに制御を戻す
        [self notifyCommandTerminated:true];
    }

@end
