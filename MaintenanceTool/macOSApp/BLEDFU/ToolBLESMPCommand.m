//
//  ToolBLESMPCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/27.
//
#import "ToolBLEHelper.h"
#import "ToolBLESMPCommand.h"
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
    // 処理成功／失敗を保持
    @property (nonatomic) bool               success;

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
        [[self toolBLEHelper] helperWillDisconnect];
    }

#pragma mark - Request and response

    - (void)doRequest {
        // 処理失敗／成功フラグをクリア
        [self setSuccess:false];
        // リクエストデータを送信
        switch ([self command]) {
            case COMMAND_BLE_DFU_GET_SLOT_INFO:
                [self doRequestGetSlotInfo];
                break;
            case COMMAND_BLE_DFU_CHANGE_TO_TEST_STATUS:
                [self doRequestChangeToTestStatus];
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
            case COMMAND_BLE_DFU_CHANGE_TO_TEST_STATUS:
                [self doResponseChangeToTestStatus];
                break;
            case COMMAND_BLE_DFU_RESET_APPLICATION:
                [self doResponseResetApplication];
                break;
            default:
                return;
        }
        // コマンドクラスに制御を戻す
        [[self delegate] bleSmpCommandDidProcess:[self command] success:[self success]
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
        // NOP
    }

    - (void)doRequestChangeToTestStatus {
        // リクエストデータを生成
        uint8_t bodyBytes[] = {
            0x02, 0x00, 0x00, 0x32, 0x00, 0x01, 0x00, 0x00, 0xbf, 0x67, 0x63, 0x6f, 0x6e, 0x66, 0x69, 0x72,
            0x6d, 0xf4, 0x64, 0x68, 0x61, 0x73, 0x68, 0x58, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff
        };
        uint16_t len = sizeof(bodyBytes);
        NSData *body = [[NSData alloc] initWithBytes:bodyBytes length:len];
        NSData *header = [self buildSmpHeader:OP_WRITE_REQ flags:0x00 len:len group:GRP_IMG_MGMT seq:0x00 id_int:CMD_IMG_MGMT_STATE];
        // リクエストデータを送信
        [self sendSmpRequestData:body withHeader:header];
    }

    - (void)doResponseChangeToTestStatus {
        // NOP
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
        // NOP
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
            [[self toolBLEHelper] helperWillDisconnect];

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
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"Incoming SMP response (%d bytes)", [responseData length]];
        [[ToolLogFile defaultLogger] hexdump:responseData];
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
            [self setSuccess:true];
            [self doResponse];
            received = 0;
            totalSize = 0;
        }
    }

    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(BLEErrorReason)reason {
        // エラーログを出力
        if (error) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"%d %@", reason, [error description]];
        } else {
            [[ToolLogFile defaultLogger] errorWithFormat:@"%d", reason];
        }
        // デバイス接続を切断
        [[self toolBLEHelper] helperWillDisconnect];
    }

    - (void)helperDidDisconnectWithError:(NSError *)error {
        // 物理切断が検知された場合は、接続復旧までその旨を保持
        if (error) {
            [self setUnexpectedDisconnection:true];
        }
        // デバイス接続の切断完了を通知
        [[self delegate] bleSmpCommandDidDisconnectWithError:error];
    }

#pragma mark - Private methods

    - (void)sendSmpRequestData:(NSData *)requestBody withHeader:(NSData *)requestHeader {
        // ヘッダーと本体を連結
        NSMutableData *sendData = [[NSMutableData alloc] initWithData:requestHeader];
        [sendData appendData:requestBody];
        // リクエストデータを送信
        [[self toolBLEHelper] helperWillWriteForCharacteristics:sendData];
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"Transmit SMP request (%d bytes)", [sendData length]];
        [[ToolLogFile defaultLogger] hexdump:sendData];
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

    - (uint16_t)getSmpResponseBodySize:(NSData *)responseData {
        // レスポンスヘッダーの３・４バイト目からデータ長を抽出
        uint8_t *bytes = (uint8_t *)[responseData bytes];
        uint16_t totalSize = ((bytes[2] << 8) & 0xff00) + (bytes[3] & 0x00ff);
        return totalSize;
    }

    - (NSString *)helperMessageOnFailConnectionWith:(BLEErrorReason)reason error:(NSError *)error {
        // BLE処理時のエラーコードを、適切なメッセージに変更する
        switch (reason) {
            case BLE_ERR_DEVICE_DISCONNECTED:
                return @"SMP service disconnected";
            default:
                return @"";
        }
    }

@end
