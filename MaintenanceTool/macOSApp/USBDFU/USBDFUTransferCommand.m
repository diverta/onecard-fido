//
//  USBDFUTransferCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#import "usb_dfu_util.h"

#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "DFUCommand.h"
#import "FIDODefines.h"
#import "USBDFUACMCommand.h"
#import "USBDFUDefine.h"
#import "USBDFUTransferCommand.h"
#import "ToolLogFile.h"

@interface USBDFUTransferCommand () <AppHIDCommandDelegate, USBDFUACMCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    @property (nonatomic) USBDFUACMCommand             *acmCommand;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;
    // 非同期処理用のキュー（転送処理用）
    @property (nonatomic) dispatch_queue_t              subQueue;

@end

@implementation USBDFUTransferCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            [self setAcmCommand:[[USBDFUACMCommand alloc] initWithDelegate:self]];
            // サブスレッドにバインドされるキューを取得
            [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.usbdfu", DISPATCH_QUEUE_SERIAL)];
        }
        return self;
    }

    - (void)terminateTransferCommand:(bool)success {
        // ステータスをクリア
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        // 上位クラスに制御を戻す
        [[self delegate] transferCommandDidTerminate:success];
    }

#pragma mark - Change to bootloader mode

    - (void)invokeTransferWithParamRef:(id)ref {
        // DFU処理のパラメーターを保持
        [self setCommandParameter:(DFUCommandParameter *)ref];
        // CTAPHID_INITから実行
        [[self appHIDCommand] doRequestCtapHidInit];
    }

    - (void)doResponseHIDCtap2Init {
        // CTAPHID_INIT応答後の処理を実行
        [self doRequestHidBootloaderMode];
    }

    - (void)doRequestHidBootloaderMode {
        // メッセージを編集し、コマンド 0xC6 を実行
        NSData *commandData = [[NSData alloc] init];
        [[self appHIDCommand] doRequestCtap2Command:COMMAND_HID_BOOTLOADER_MODE withCMD:HID_CMD_BOOTLOADER_MODE withData:commandData];
    }

    - (void)doResponseHidBootloaderMode:(uint8_t)cmd response:(NSData *)response {
        // ブートローダーモード遷移コマンド成功時
        if (cmd == HID_CMD_BOOTLOADER_MODE) {
            // 処理ステータスを設定 --> USB切断検知により処理続行
            [[self commandParameter] setDfuStatus:DFU_ST_TO_BOOTLOADER_MODE];

        } else {
            // ブートローダーモード遷移コマンド失敗時は、即時で制御を戻す
            [[self delegate] notifyErrorMessage:MSG_DFU_TARGET_NOT_BOOTLOADER_MODE];
            [self terminateTransferCommand:false];
        }
    }

#pragma mark - DFU transfer main process

    - (void)performTransferProcess {
        // DFU実行開始を通知
        [[self delegate] notifyProgress:MSG_DFU_PROCESS_TRANSFER_IMAGE progressValue:0];
        dispatch_async([self subQueue], ^{
            // DFUを実行
            bool ret = [self performTransferProcessSync];
            // DFU対象デバイスから切断
            [[self acmCommand] closeACMConnection];
            if (ret == false) {
                // 処理失敗時は、処理進捗画面に対し通知
                [[self delegate] notifyErrorMessage:MSG_DFU_IMAGE_TRANSFER_FAILED];
                [self terminateTransferCommand:false];
            } else {
                // 処理成功時は、再接続まで待機 --> HID接続検知により、バージョン更新判定に遷移
                [[self delegate] notifyProgress:MSG_DFU_PROCESS_WAITING_UPDATE progressValue:100];
                [[ToolLogFile defaultLogger] info:MSG_DFU_IMAGE_TRANSFER_SUCCESS];
                [[self commandParameter] setDfuStatus:DFU_ST_WAIT_FOR_BOOT];
            }
        });
    }

    - (bool)performTransferProcessSync {
        // DFU対象デバイスの通知設定
        if ([self sendSetReceiptRequest] == false) {
            return false;
        }
        // DFU対象デバイスからMTUを取得
        if ([self sendGetMtuRequest] == false) {
            return false;
        }
        // TODO: 仮の実装です。
        [NSThread sleepForTimeInterval:3.0];
        return true;
    }

#pragma mark - Sub process

    - (bool)sendSetReceiptRequest {
        // SET RECEIPT 02 00 00 C0 -> 60 02 01 C0
        static uint8_t request[] = {
            NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [[self acmCommand] assertDFUResponseSuccess:response];
    }

    - (bool)sendGetMtuRequest {
        // Get the preferred MTU size on the request.
        // GET MTU 07 C0 -> 60 07 01 83 00 C0
        static uint8_t mtuRequest[] = {NRF_DFU_OP_MTU_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:mtuRequest length:sizeof(mtuRequest)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([[self acmCommand] assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスからMTUを取得（4〜5バイト目）
        uint16_t mtu = [self convertLEBytesToUint16:[response bytes] offset:3];
        size_t mtu_size = usb_dfu_object_set_mtu(mtu);
        [[ToolLogFile defaultLogger] debugWithFormat:@"ToolDFUCommand: MTU=%d", mtu_size];
        return true;
    }

    - (uint16_t)convertLEBytesToUint16:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
        return uint;
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
        if ([[self commandParameter] dfuStatus] != DFU_ST_TO_BOOTLOADER_MODE) {
            return;
        }
        // ステータスをクリア
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        dispatch_async([self subQueue], ^{
            // USB DFUに必要なACM接続を確立
            [[self acmCommand] establishACMConnection];
        });
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [[ToolLogFile defaultLogger] error:errorMessage];
            [self terminateTransferCommand:false];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_CTAP2_INIT:
                [self doResponseHIDCtap2Init];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self doResponseHidBootloaderMode:cmd response:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self terminateTransferCommand:false];
                break;
        }
    }

#pragma mark - Call back from USBDFUACMCommand

    - (void)didEstablishACMConnection:(bool)success {
        [self performTransferProcess];
    }

@end
