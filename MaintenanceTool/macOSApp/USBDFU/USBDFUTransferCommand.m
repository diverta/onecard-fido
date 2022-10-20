//
//  USBDFUTransferCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#import "nrf52_app_image.h"
#import "usb_dfu_util.h"

#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "DFUCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "USBDFUACMCommand.h"
#import "USBDFUDefine.h"
#import "USBDFUTransferCommand.h"

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
        // datイメージを転送
        if ([self transferDFUImage:NRF_DFU_BYTE_OBJ_INIT_CMD
                         imageData:nrf52_app_image_dat()
                         imageSize:nrf52_app_image_dat_size()] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debug:@"USBDFUTransferCommand: update init command object done"];
        // binイメージを転送
        if ([self transferDFUImage:NRF_DFU_BYTE_OBJ_DATA
                         imageData:nrf52_app_image_bin()
                         imageSize:nrf52_app_image_bin_size()] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debug:@"USBDFUTransferCommand: update data object done"];
        return true;
    }

#pragma mark - DFU transfer sub process

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
        [[ToolLogFile defaultLogger] debugWithFormat:@"USBDFUTransferCommand: MTU=%d", mtu_size];
        return true;
    }

    - (bool)transferDFUImage:(uint8_t)objectType imageData:(uint8_t *)data imageSize:(size_t)size {
        // １回あたりの送信データ最大長を取得
        size_t maxCreateSize;
        if ([self sendSelectObjectRequest:objectType pMaxCreateSize:&maxCreateSize] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"USBDFUTransferCommand: object select size=%d, create size max=%d", size, maxCreateSize];
        // データを分割送信
        size_t remaining = size;
        size_t alreadySent = 0;
        while (remaining > 0) {
            // 送信サイズを通知
            size_t sendSize = (maxCreateSize < remaining) ? maxCreateSize : remaining;
            if ([self sendCreateObjectRequest:objectType imageSize:sendSize] == false) {
                return false;
            }
            // データを送信
            uint8_t *sendData = data + alreadySent;
            if ([self sendWriteCommandObjectRequest:objectType
                                          imageData:sendData imageSize:sendSize] == false) {
                return false;
            }
            // 送信データのチェックサムを検証
            alreadySent += sendSize;
            if ([self sendGetCrcRequest:objectType imageSize:alreadySent] == false) {
                return false;
            }
            // 送信データをコミット
            if ([self sendExecuteObjectRequest] == false) {
                return false;
            }
            // 進捗画面に通知
            if (objectType == NRF_DFU_BYTE_OBJ_DATA) {
                // 転送比率を計算
                int percentage = (int)(alreadySent * 100 / size);
                // 転送状況を画面表示
                NSString *progress = [[NSString alloc] initWithFormat:MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage];
                [[self delegate] notifyProgress:progress progressValue:percentage];
            }
            // 未送信サイズを更新
            remaining -= sendSize;
        }
        return true;
    }

    - (bool)sendSelectObjectRequest:(uint8_t)objectType pMaxCreateSize:(size_t *)pMaxCreateSize {
        // SELECT OBJECT 06 xx C0 -> 60 06 xx 00 01 00 00 00 00 00 00 00 00 00 00 C0
        uint8_t request[] = {
            NRF_DFU_OP_OBJECT_SELECT, objectType,
            NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([[self acmCommand] assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスから、イメージの最大送信可能サイズを取得（4〜7バイト目）
        *pMaxCreateSize = (size_t)[self convertLEBytesToUint32:[response bytes] offset:3];
        // チェックサムを初期化
        usb_dfu_object_checksum_reset();
        return true;
    }

    - (bool)sendCreateObjectRequest:(uint8_t)objectType imageSize:(size_t)imageSize {
        // CREATE OBJECT 01 xx 87 00 00 00 C0 -> 60 01 01 C0
        uint8_t createObjectRequest[] = {
            NRF_DFU_OP_OBJECT_CREATE, objectType, 0x00, 0x00, 0x00, 0x00,
            NRF_DFU_BYTE_EOM};
        uint32_t commandObjectLen = (uint32_t)imageSize;
        [self convertUint32ToLEBytes:commandObjectLen data:createObjectRequest offset:2];
        
        NSData *data = [NSData dataWithBytes:createObjectRequest length:sizeof(createObjectRequest)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [[self acmCommand] assertDFUResponseSuccess:response];
    }

    - (bool)sendWriteCommandObjectRequest:(uint8_t)objectType
                                imageData:(uint8_t *)data imageSize:(size_t)size {
        // オブジェクト種別に対応するデータ／サイズを設定
        usb_dfu_object_frame_init(data, size);
        // 送信フレームを生成
        while (usb_dfu_object_frame_prepare()) {
            // フレームを送信
            NSData *frame = [NSData dataWithBytes:usb_dfu_object_frame_data()
                                          length:usb_dfu_object_frame_size()];
            if ([[self acmCommand] sendRequestData:frame] == false) {
                return false;
            }
#if CDC_ACM_LOG_DEBUG
            [[ToolLogFile defaultLogger] debugWithFormat:@"CDC ACM Send (%d bytes)", [frame length]];
#endif
        }
        return true;
    }

    - (bool)sendGetCrcRequest:(uint8_t)objectType imageSize:(size_t)imageSize {
        // CRC GET 03 C0 -> 60 03 01 87 00 00 00 38 f4 97 72 C0
        uint8_t request[] = {NRF_DFU_OP_CRC_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([[self acmCommand] assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスデータから、エスケープシーケンスを取り除く
        NSData *respUnesc = [self unescapeResponseData:response];

        // 送信データ長を検証
        size_t recvSize = (size_t)[self convertLEBytesToUint32:[respUnesc bytes] offset:3];
        if (recvSize != imageSize) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"USBDFUTransferCommand: send object %d failed (expected %d bytes, recv %d bytes)",
             objectType, imageSize, recvSize];
            return false;
        }
        // チェックサムを検証
        uint32_t checksum = [self convertLEBytesToUint32:[respUnesc bytes] offset:7];
        if (checksum != usb_dfu_object_checksum_get()) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"USBDFUTransferCommand: send object %d failed (checksum error)",
             objectType];
            return false;
        }
        return true;
    }

    - (bool)sendExecuteObjectRequest {
        // EXECUTE OBJECT 04 C0 -> 60 04 01 C0
        static uint8_t request[] = {NRF_DFU_OP_OBJECT_EXECUTE, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [[self acmCommand] sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [[self acmCommand] assertDFUResponseSuccess:response];
    }

    - (void)convertUint32ToLEBytes:(uint32_t)uint data:(uint8_t *)data offset:(uint16_t)offset {
        uint8_t *bytes = data + offset;
        for (uint8_t i = 0; i < 4; i++) {
            *bytes++ = uint & 0xff;
            uint = uint >> 8;
        }
    }

    - (uint32_t)convertLEBytesToUint32:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint32_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8)
            | ((uint32_t)bytes[offset + 2] << 16) | ((uint32_t)bytes[offset + 3] << 24);
        return uint;
    }

    - (uint16_t)convertLEBytesToUint16:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
        return uint;
    }

    - (NSData *)unescapeResponseData:(NSData *)response {
        uint8_t c;
        NSMutableData *unescaped = [[NSMutableData alloc] init];
        
        uint8_t *data = (uint8_t *)[response bytes];
        size_t   size = [response length];
        
        bool escapeChar = false;
        for (size_t i = 0; i < size; i++) {
            c = data[i];
            if (c == 0xdb) {
                escapeChar = true;
            } else {
                if (escapeChar) {
                    escapeChar = false;
                    if (c == 0xdc) {
                        c = 0xc0;
                    } else if (c == 0xdd) {
                        c = 0xdb;
                    }
                }
                [unescaped appendBytes:&c length:sizeof(c)];
            }
        }
        return unescaped;
    }

#pragma mark - Version checking process

    - (void)doRequestHIDGetVersionInfo {
        // ステータスを更新（更新後バージョン照会）
        [[self commandParameter] setDfuStatus:DFU_ST_CHECK_UPDATE_VERSION];
        // HID経由でFlash ROM情報を取得（コマンド 0xC3 を実行、メッセージ無し）
        [[self appHIDCommand] doRequestCommand:COMMAND_HID_GET_VERSION_INFO withCMD:HID_CMD_GET_VERSION_INFO withData:nil];
    }

    - (void)doResponseHIDGetVersionInfo:(NSData *)versionInfoResponse {
        // 更新後バージョン照会の場合（ファームウェア反映待ち完了時）
        if ([[self commandParameter] dfuStatus] == DFU_ST_CHECK_UPDATE_VERSION) {
            [self notifyFirmwareVersionForComplete:versionInfoResponse];
        }
    }

    - (void)notifyFirmwareVersionForComplete:(NSData *)response {
        if (response == nil || [response length] < 2) {
            // 処理失敗時は、処理進捗画面に対し通知
            [[self delegate] notifyErrorMessage:MSG_DFU_VERSION_INFO_GET_FAILED];
            [self terminateTransferCommand:false];
            return;
        }
        // 戻りメッセージからバージョン情報を抽出
        [self extractVersionAndBoardnameFrom:response];
        // バージョン情報を比較して終了判定
        bool result = [self compareUpdateVersion:[[self commandParameter] currentVersion]];
        // 処理進捗画面に対し、処理結果を通知する
        [self terminateTransferCommand:result];
    }

    - (void)extractVersionAndBoardnameFrom:(NSData *)response {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:response];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 取得したバージョン情報を内部保持
        [[self commandParameter] setCurrentVersion:array[1]];
        [[self commandParameter] setCurrentBoardname:array[2]];
    }

    - (bool)compareUpdateVersion:(NSString *)update {
        // バージョン情報を比較
        char *fw_version = nrf52_app_image_zip_version();
        NSString *expected = [[NSString alloc] initWithUTF8String:fw_version];
        if (strcmp([update UTF8String], fw_version) == 0) {
            // バージョンが同じであればDFU処理は正常終了とする
            NSString *infoMessage = [[NSString alloc] initWithFormat:MSG_DFU_FIRMWARE_VERSION_UPDATED, expected];
            [[self delegate] notifyInfoMessage:infoMessage];
            return true;
        } else {
            // バージョンが同じでなければ異常終了とする
            NSString *errorMessage = [[NSString alloc] initWithFormat:MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED, expected];
            [[self delegate] notifyErrorMessage:errorMessage];
            return false;
        }
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
        if ([[self commandParameter] dfuStatus] != DFU_ST_WAIT_FOR_BOOT) {
            return;
        }
        // ステータスをクリア
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        dispatch_async([self subQueue], ^{
            // HID経由で更新後のバージョン情報を取得
            [self doRequestHIDGetVersionInfo];
        });
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
            case COMMAND_HID_GET_VERSION_INFO:
                [self doResponseHIDGetVersionInfo:response];
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
