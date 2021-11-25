//
//  ToolBLESMPCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/27.
//
#import "BLEDFUDefine.h"
#import "ToolBLEHelper.h"
#import "ToolBLESMPCommand.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

#define SmpServiceUUID          @"8D53DC1D-1DB7-4CD3-868B-8A527460AA84"
#define SmpCharacteristicUUID   @"DA2E7828-FBCE-4E01-AE9E-261174997C48"

#define OP_READ_REQ             0
#define OP_READ_RES             1
#define OP_WRITE_REQ            2
#define OP_WRITE_RES            3

#define GRP_IMG_MGMT            1
#define CMD_IMG_MGMT_STATE      0
#define CMD_IMG_MGMT_UPLOAD     1

#define GRP_OS_MGMT             0
#define CMD_OS_MGMT_RESET       5

#define SMP_HEADER_SIZE         8

@interface ToolBLESMPCommand () <ToolBLEHelperDelegate>

    // BLE接続に関する情報を保持
    @property (nonatomic) ToolBLEHelper     *toolBLEHelper;
    // 呼び出し元のコマンドオブジェクト参照を保持
    @property (nonatomic, weak) id           toolCommandRef;
    // コマンドを保持
    @property (nonatomic) Command            command;
    // リクエスト、レスポンスを保持
    @property (nonatomic) NSData            *requestData;
    @property (nonatomic) NSMutableData     *responseData;
    // 物理接続が切れた旨を保持
    @property (nonatomic) bool               unexpectedDisconnection;
    // デバイス接続の切断理由を保持
    @property (nonatomic) bool               disconnectByError;

@end

@implementation ToolBLESMPCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolBLESMPCommandDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self setToolBLEHelper:[[ToolBLEHelper alloc] initWithDelegate:self]];
        }
        return self;
    }

#pragma mark - Public methods

    - (void)commandWillConnect {
        // BLEデバイス接続処理を開始する
        [self setUnexpectedDisconnection:false];
        [[self toolBLEHelper] helperWillConnectWithUUID:SmpServiceUUID];
    }

    - (void)commandWillProcess:(Command)command request:(NSData *)request forCommand:(id)commandRef {
        // コマンド、上位クラスの参照を保持
        [self setToolCommandRef:commandRef];
        [self setCommand:command];
        [self setRequestData:request];
        // リクエストデータを送信
        [self doRequest];
    }

    - (void)commandWillDisconnect {
        // デバイス接続を切断
        [self doDisconnectByError:false];
    }

#pragma mark - Request and response

    - (void)doRequest {
        // リクエストデータを送信
        switch ([self command]) {
            case COMMAND_BLE_DFU_GET_SLOT_INFO:
                [self doRequestGetSlotInfo];
                break;
            case COMMAND_BLE_DFU_UPLOAD_IMAGE:
                [self doRequestUploadImage];
                break;
            case COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE:
                [self doRequestChangeImageUpdateMode];
                break;
            case COMMAND_BLE_DFU_RESET_APPLICATION:
                [self doRequestResetApplication];
                break;
            default:
                break;
        }
    }

    - (void)doResponse {
        // レスポンスを処理
        switch ([self command]) {
            case COMMAND_BLE_DFU_GET_SLOT_INFO:
                [self doResponseGetSlotInfo];
                break;
            case COMMAND_BLE_DFU_UPLOAD_IMAGE:
                [self doResponseUploadImage];
                break;
            case COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE:
                [self doResponseChangeImageUpdateMode];
                break;
            case COMMAND_BLE_DFU_RESET_APPLICATION:
                [self doResponseResetApplication];
                break;
            default:
                return;
        }
    }

    - (void)commandDidProcess:(bool)success {
        // コマンドクラスに制御を戻す
        [[self delegate] bleSmpCommandDidProcess:[self command] success:success
                                        response:[self responseData] forCommand:[self toolCommandRef]];
    }

    - (void)doRequestGetSlotInfo {
        // リクエストデータを生成
        uint8_t bodyBytes[] = {0xbf, 0xff};
        uint16_t len = sizeof(bodyBytes);
        NSData *body = [[NSData alloc] initWithBytes:bodyBytes length:len];
        NSData *header = [self buildSmpHeader:OP_READ_REQ flags:0x00 len:len group:GRP_IMG_MGMT seq:0x00 id_int:CMD_IMG_MGMT_STATE];
        // リクエストデータを送信
        [self sendSmpRequestData:body withHeader:header];
    }

    - (void)doResponseGetSlotInfo {
        // コマンドクラスに制御を戻す
        [self commandDidProcess:true];
    }

    - (void)doRequestUploadImage {
        // リクエストデータを生成
        NSData *body = [self generateBodyForRequestUploadImage:[self requestData]];
        NSData *header = [self buildSmpHeader:OP_WRITE_REQ flags:0x00 len:[body length] group:GRP_IMG_MGMT seq:0x00 id_int:CMD_IMG_MGMT_UPLOAD];
        // リクエストデータを送信
        [self sendSmpRequestData:body withHeader:header];
    }

    - (void)doResponseUploadImage {
        // コマンドクラスに制御を戻す
        [self commandDidProcess:true];
    }

    - (void)doRequestChangeImageUpdateMode {
        // リクエストデータを生成
        uint8_t bodyBytes[] = {
            0xbf, 0x67, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x72, 0x6d, 0x00,
            0x64, 0x68, 0x61, 0x73, 0x68, 0x58, 0x20
        };
        // イメージ反映モードを設定（confirm=false/true）
        if (IMAGE_UPDATE_TEST_MODE) {
            bodyBytes[9] = 0xf4;
        } else {
            bodyBytes[9] = 0xf5;
        }
        uint16_t len = sizeof(bodyBytes);
        NSMutableData *body = [[NSMutableData alloc] initWithBytes:bodyBytes length:len];
        // 本体にSHA-256ハッシュを連結
        [body appendData:[self requestData]];
        // 終端文字を連結
        [self appendSmpBodyTerminatorByteTo:body];
        // ヘッダーを生成
        NSData *header = [self buildSmpHeader:OP_WRITE_REQ flags:0x00 len:[body length] group:GRP_IMG_MGMT seq:0x00 id_int:CMD_IMG_MGMT_STATE];
        // リクエストデータを送信
        [self sendSmpRequestData:body withHeader:header];
    }

    - (void)doResponseChangeImageUpdateMode {
        // コマンドクラスに制御を戻す
        [self commandDidProcess:true];
    }

    - (void)doRequestResetApplication {
        // リクエストデータを生成
        uint8_t bodyBytes[] = {0xbf, 0xff};
        uint16_t len = sizeof(bodyBytes);
        NSData *body = [[NSData alloc] initWithBytes:bodyBytes length:len];
        NSData *header = [self buildSmpHeader:OP_WRITE_REQ flags:0x00 len:len group:GRP_OS_MGMT seq:0x00 id_int:CMD_OS_MGMT_RESET];
        // リクエストデータを送信
        [self sendSmpRequestData:body withHeader:header];
    }

    - (void)doResponseResetApplication {
        // コマンドクラスに制御を戻す
        [self commandDidProcess:true];
    }

#pragma mark - Callback from ToolBLEHelper

    - (void)helperDidScanForPeripheral:(id)peripheralRef withUUID:(NSString *)uuidString {
        // スキャンされたサービスUUIDを比較し、同じであればペリフェラル接続を試行
        if ([uuidString isEqualToString:SmpServiceUUID]) {
            [[self toolBLEHelper] helperWillConnectPeripheral:peripheralRef];
        }
    }

    - (void)helperDidConnectPeripheral {
        if ([self unexpectedDisconnection]) {
            // 物理接続断によるエラー発生後の接続復旧である場合は、このクラスの論理接続を破棄
            [self setUnexpectedDisconnection:false];
            [self doDisconnectByError:false];

        } else {
            // SMPサービスUUIDによる接続検知の場合は、SMPサービス接続を試行
            [[ToolLogFile defaultLogger] info:@"SMP server scanned"];
            [[self toolBLEHelper] helperWillDiscoverServiceWithUUID:SmpServiceUUID];
        }
    }

    - (void)helperDidDiscoverService {
        // SMPキャラクタリスティックに接続
        [[ToolLogFile defaultLogger] info:@"SMP service discovered"];
        NSArray<NSString *> *characteristicUUIDs = @[SmpCharacteristicUUID];
        [[self toolBLEHelper] helperWillDiscoverCharacteristicsWithUUIDs:characteristicUUIDs];
    }

    - (void)helperDidDiscoverCharacteristics {
        // 接続を通知
        [[ToolLogFile defaultLogger] info:@"SMP service connected"];
        [[self delegate] bleSmpCommandDidConnect];
    }

    - (void)helperDidWriteForCharacteristics {
        // レスポンス待ち（レスポンスタイムアウト監視開始）
        [[self toolBLEHelper] helperWillReadForCharacteristics];
    }

    - (void)helperDidReadForCharacteristic:(NSData *)responseData {
        // ログ出力
        if ([self command] != COMMAND_BLE_DFU_UPLOAD_IMAGE) {
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"Incoming SMP response (%d bytes)", [responseData length]];
            [[ToolLogFile defaultLogger] hexdump:responseData];
        }
        // 受信済みバイト数を保持
        static uint16_t received = 0;
        static uint16_t totalSize = 0;
        // 受信したレスポンスデータを保持
        uint16_t responseSize = (uint16_t)[responseData length];
        if (received == 0) {
            // レスポンスヘッダーからデータ長を抽出
            totalSize = [self getSmpResponseBodySize:responseData];
            // 受信済みデータを保持
            received = responseSize - SMP_HEADER_SIZE;
            NSData *responseBody = [responseData subdataWithRange:NSMakeRange(SMP_HEADER_SIZE, received)];
            [self setResponseData:[[NSMutableData alloc] initWithData:responseBody]];
        } else {
            // 受信済みデータに連結
            received += responseSize;
            [[self responseData] appendData:responseData];
        }
        // 全フレームを受信したら、レスポンス処理を実行
        if (received == totalSize) {
            [self doResponse];
            received = 0;
            totalSize = 0;
        }
    }

    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(BLEErrorReason)reason {
        // エラーログを出力
        NSString *message = [self helperMessageOnFailConnectionWith:reason error:error];
        if (error) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"%@ %@", message, [error description]];
        } else {
            [[ToolLogFile defaultLogger] error:message];
        }
        // デバイス接続を切断
        [self doDisconnectByError:true];
    }

    - (void)helperDidDisconnectWithError:(NSError *)error {
        // 物理切断が検知された場合は、接続復旧までその旨を保持
        if (error) {
            [self setUnexpectedDisconnection:true];
        } else {
            [[ToolLogFile defaultLogger] info:@"SMP service disconnected"];
        }
        if ([self disconnectByError]) {
            // レスポンスエラーとしてコマンドクラスに通知
            [self setDisconnectByError:false];
            [self commandDidProcess:false];
        } else {
            // 切断をコマンドクラスに通知
            [[self delegate] bleSmpCommandDidDisconnectWithError:error];
        }
    }

#pragma mark - Private methods

    - (void)doDisconnectByError:(bool)b {
        [self setDisconnectByError:b];
        [[self toolBLEHelper] helperWillDisconnect];
    }

    - (void)sendSmpRequestData:(NSData *)requestBody withHeader:(NSData *)requestHeader {
        // ヘッダーと本体を連結
        NSMutableData *sendData = [[NSMutableData alloc] initWithData:requestHeader];
        [sendData appendData:requestBody];
        // リクエストデータを送信
        [[self toolBLEHelper] helperWillWriteForCharacteristics:sendData];
        // ログ出力
        if ([self command] != COMMAND_BLE_DFU_UPLOAD_IMAGE) {
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"Transmit SMP request (%d bytes)", [sendData length]];
            [[ToolLogFile defaultLogger] hexdump:sendData];
        }
    }

    - (NSData *)buildSmpHeader:(uint8_t)op flags:(uint8_t)flags len:(uint16_t)len
                         group:(uint16_t)group seq:(uint8_t)seq id_int:(uint8_t)id_int {
        uint8_t header[] = {
            op,
            flags,
            (uint8_t)(len >> 8),   (uint8_t)(len & 0xff),
            (uint8_t)(group >> 8), (uint8_t)(group & 0xff),
            seq,
            id_int
        };
        NSData *data = [[NSData alloc] initWithBytes:header length:sizeof(header)];
        return data;
    }

    - (void)appendSmpBodyTerminatorByteTo:(NSMutableData *)bodyData {
        // 終端文字を連結
        uint8_t bodyTerminator[] = {0xff};
        NSData *terminator = [[NSData alloc] initWithBytes:bodyTerminator length:sizeof(bodyTerminator)];
        [bodyData appendData:terminator];
    }

    - (uint16_t)getSmpResponseBodySize:(NSData *)responseData {
        // レスポンスヘッダーの３・４バイト目からデータ長を抽出
        uint8_t *bytes = (uint8_t *)[responseData bytes];
        uint16_t totalSize = ((bytes[2] << 8) & 0xff00) + (bytes[3] & 0x00ff);
        return totalSize;
    }

    - (NSString *)helperMessageOnFailConnectionWith:(BLEErrorReason)reason error:(NSError *)error {
        // BLE処理時のエラーコードを、適切なメッセージに変更する
        switch (reason) {
            case BLE_ERR_BLUETOOTH_OFF:
                return MSG_BLE_PARING_ERR_BT_OFF;
            case BLE_ERR_DEVICE_CONNECT_FAILED:
                return MSG_U2F_DEVICE_CONNECT_FAILED;
            case BLE_ERR_DEVICE_CONNREQ_TIMEOUT:
                return MSG_U2F_DEVICE_CONNREQ_TIMEOUT;
            case BLE_ERR_DEVICE_SCAN_TIMEOUT:
                return MSG_U2F_DEVICE_SCAN_TIMEOUT;
            case BLE_ERR_SERVICE_NOT_DISCOVERED:
                return MSG_BLE_SERVICE_NOT_DISCOVERED;
            case BLE_ERR_SERVICE_NOT_FOUND:
                return @"SMP service not found";
            case BLE_ERR_DISCOVER_SERVICE_TIMEOUT:
                return @"SMP service discover timed out";
            case BLE_ERR_CHARACT_NOT_DISCOVERED:
                return @"SMP characteristic not found";
            case BLE_ERR_DISCOVER_CHARACT_TIMEOUT:
                return @"SMP characteristic discover timed out";
            case BLE_ERR_CHARACT_NOT_EXIST:
                return @"SMP characteristic not exist";
            case BLE_ERR_NOTIFICATION_FAILED:
                return @"SMP characteristic notification failed";
            case BLE_ERR_NOTIFICATION_STOP:
                return MSG_BLE_NOTIFICATION_STOP;
            case BLE_ERR_SUBSCRIBE_CHARACT_TIMEOUT:
                return @"SMP characteristic subscription timed out";
            case BLE_ERR_REQUEST_SEND_FAILED:
                return MSG_REQUEST_SEND_FAILED;
            case BLE_ERR_RESPONSE_RECEIVE_FAILED:
                return MSG_RESPONSE_RECEIVE_FAILED;
            case BLE_ERR_REQUEST_TIMEOUT:
                return MSG_REQUEST_TIMEOUT;
            default:
                return MSG_OCCUR_BLECONN_ERROR;
        }
    }

#pragma mark - Transfer image data request

    - (NSData *)generateBodyForRequestUploadImage:(NSData *)imageDataTotal {
        // リクエストデータ
        uint8_t bodyBytes[] = {0xbf};
        NSMutableData *body = [[NSMutableData alloc] initWithBytes:bodyBytes length:sizeof(bodyBytes)];
        if ([self imageBytesSent] == 0) {
            // 初回呼び出しの場合、イメージ長を設定
            [body appendData:[self generateLenBytes:[imageDataTotal length]]];
            // イメージのハッシュ値を設定
            NSData *hash = [ToolCommon generateSHA256HashDataOf:imageDataTotal];
            [body appendData:[self generateShaBytes:hash]];
        }
        // 転送済みバイト数を設定
        [body appendData:[self generateOffBytes:[self imageBytesSent]]];
        // 転送イメージを連結（データ本体が240バイトに収まるよう、上限サイズを事前計算）
        size_t remainingSize = 240 - [body length] - 1;
        [body appendData:[self generateDataBytes:imageDataTotal bytesSent:[self imageBytesSent] remainingSize:remainingSize]];
        // 終端文字を設定
        uint8_t bodyTerminator[] = {0xff};
        NSData *terminator = [[NSData alloc] initWithBytes:bodyTerminator length:sizeof(bodyTerminator)];
        [body appendData:terminator];
        return body;
    }

    - (NSData *)generateDataBytes:(NSData *)imageData bytesSent:(size_t)bytesSent remainingSize:(size_t)remaining {
        // 転送バイト数を設定
        uint8_t bodyBytes[] = {
            0x64, 0x64, 0x61, 0x74, 0x61, 0x58, 0x00
        };
        // 転送バイト数
        size_t bytesToSend = remaining - sizeof(bodyBytes);
        if (bytesToSend > [imageData length] - bytesSent) {
            bytesToSend = [imageData length] - bytesSent;
        }
        bodyBytes[6] = (uint8_t)bytesToSend;
        // 転送イメージを抽出
        NSData *sendData = [imageData subdataWithRange:NSMakeRange(bytesSent, bytesToSend)];
        // 転送イメージを連結
        NSMutableData *body = [[NSMutableData alloc] initWithBytes:bodyBytes length:sizeof(bodyBytes)];
        [body appendData:sendData];
        return body;
    }

    - (NSData *)generateLenBytes:(size_t)bytesTotal {
        // イメージ長を設定
        uint8_t lenBytes[] = {
            0x63, 0x6c, 0x65, 0x6e, 0x1a, 0x00, 0x00, 0x00, 0x00
        };
        [ToolCommon setLENumber32:(uint32_t)bytesTotal toBEBytes:(lenBytes + 5)];
        NSData *lenData = [[NSData alloc] initWithBytes:lenBytes length:sizeof(lenBytes)];
        return lenData;
    }

    - (NSData *)generateShaBytes:(NSData *)hashBytes {
        // イメージのハッシュ値を設定
        uint8_t shaBytes[] = {
            0x63, 0x73, 0x68, 0x61, 0x43, 0x00, 0x00, 0x00,
        };
        // 指定領域から３バイト分の領域に、SHA-256ハッシュの先頭３バイト分を設定
        uint8_t *bytes = (uint8_t *)[hashBytes bytes];
        memcpy(shaBytes + 5, bytes, 3);
        NSData *shaData = [[NSData alloc] initWithBytes:shaBytes length:sizeof(shaBytes)];
        return shaData;
    }

    - (NSData *)generateOffBytes:(size_t)bytesSent {
        // 転送済みバイト数を設定
        uint8_t offBytes[] = {
            0x63, 0x6f, 0x66, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        NSUInteger len = sizeof(offBytes);
        if (bytesSent == 0) {
            len = 5;
        } else if (bytesSent < 0x100) {
            offBytes[4] = 0x18;
            offBytes[5] = (uint8_t)bytesSent;
            len = 6;
        } else if (bytesSent < 0x10000) {
            offBytes[4] = 0x19;
            [ToolCommon setLENumber16:(uint16_t)bytesSent toBEBytes:(offBytes + 5)];
            len = 7;
        } else {
            offBytes[4] = 0x1a;
            [ToolCommon setLENumber32:(uint32_t)bytesSent toBEBytes:(offBytes + 5)];
        }
        NSData *offData = [[NSData alloc] initWithBytes:offBytes length:len];
        return offData;
    }

@end
