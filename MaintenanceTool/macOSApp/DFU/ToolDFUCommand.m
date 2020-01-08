//
//  ToolDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#import <Foundation/Foundation.h>

#import "debug_log.h"
#import "nrf52_app_image.h"
#import "ToolCDCHelper.h"
#import "ToolDFUCommand.h"
#import "ToolLogFile.h"

// DFU対象ファイル名
#define NRF52_APP_DAT_FILE_NAME @"nrf52840_xxaa.dat"
#define NRF52_APP_BIN_FILE_NAME @"nrf52840_xxaa.bin"

// 応答タイムアウト
#define TIMEOUT_SEC_DFU_PING_RESPONSE  1.0
#define TIMEOUT_SEC_DFU_OPER_RESPONSE  3.0

// 詳細ログ出力
#define CDC_ACM_LOG_DEBUG false

@interface ToolDFUCommand ()

    @property (nonatomic) ToolCDCHelper *toolCDCHelper;

@end

@implementation ToolDFUCommand

    - (id)init {
        self = [super init];
        
        // ToolCDCHelperのインスタンスを生成
        [self setToolCDCHelper:[[ToolCDCHelper alloc] init]];
        return self;
    }

    - (NSString *)testMain {
        // ファームウェアのイメージファイル（.dat／.bin）から、バイナリーイメージを読込
        if ([self readDFUImages] == false) {
            return @"DFUに必要なアプリケーションイメージが同梱されていません.";
        }
        // DFU対象デバイスに接続
        NSString *ACMDevicePath = [self getConnectedDevicePath];
        if (ACMDevicePath == nil) {
            return @"DFU対象デバイスが接続されていません.";
        }
        // DFUを実行
        bool ret = [self performDFU];
        // 処理完了
        [[self toolCDCHelper] disconnectDevice];
        return (ret ? @"DFUが成功しました." : @"DFUが失敗しました.");
    }

    - (bool)readDFUImages {
        // .datファイルからイメージを読込
        if (nrf52_app_image_dat_read([self getBundleResourcePathChar:NRF52_APP_DAT_FILE_NAME]) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolDFUCommand: %s", log_debug_message()];
            return false;
        }
        // .binファイルからイメージを読込
        if (nrf52_app_image_bin_read([self getBundleResourcePathChar:NRF52_APP_BIN_FILE_NAME]) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolDFUCommand: %s", log_debug_message()];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: %@(%d bytes), %@(%d bytes)",
         NRF52_APP_DAT_FILE_NAME, nrf52_app_image_dat_size(),
         NRF52_APP_BIN_FILE_NAME, nrf52_app_image_bin_size()];
        return true;
    }

    - (bool)performDFU {
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
        [[ToolLogFile defaultLogger] debug:@"ToolDFUCommand: update init command object done"];
        // binイメージを転送
        if ([self transferDFUImage:NRF_DFU_BYTE_OBJ_DATA
                         imageData:nrf52_app_image_bin()
                         imageSize:nrf52_app_image_bin_size()] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debug:@"ToolDFUCommand: update data object done"];
        return true;
    }

    - (bool)transferDFUImage:(uint8_t)objectType
                   imageData:(uint8_t *)data imageSize:(size_t)size {
        // １回あたりの送信データ最大長を取得
        size_t maxCreateSize;
        if ([self sendSelectObjectRequest:objectType pMaxCreateSize:&maxCreateSize] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: object select size=%d, create size max=%d",
         size, maxCreateSize];
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
            // 未送信サイズを更新
            remaining -= sendSize;
        }
        return true;
    }

    - (const char *)getBundleResourcePathChar:(NSString *)fileName {
        // リソースバンドル・ディレクトリーの絶対パスを取得
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        // 引数のファイル名を、ディレクトリーパスに連結
        NSString *resourcePathStr = [[NSString alloc] initWithFormat:@"%@/%@", resourcePath, fileName];
        // 作成されたファルパスを、const char形式に変換
        const char *resourcePathChar = (const char *)[resourcePathStr UTF8String];
        
        return resourcePathChar;
    }

    - (bool)sendPingRequest:(uint8_t)id {
        // PING 09 01 C0 -> 60 09 01 01 C0
        uint8_t pingRequest[] = {NRF_DFU_OP_PING, id, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:pingRequest length:sizeof(pingRequest)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_PING_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // IDを比較
        uint8_t *pingResponse = (uint8_t *)[response bytes];
        return (pingResponse[3] == id);
    }

    - (bool)sendSetReceiptRequest {
        // SET RECEIPT 02 00 00 C0 -> 60 02 01 C0
        static uint8_t request[] = {
            NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
    }

    - (bool)sendGetMtuRequest {
        // Get the preferred MTU size on the request.
        // GET MTU 07 C0 -> 60 07 01 83 00 C0
        static uint8_t mtuRequest[] = {NRF_DFU_OP_MTU_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:mtuRequest length:sizeof(mtuRequest)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスからMTUを取得（4〜5バイト目）
        uint16_t mtu = [self convertLEBytesToUint16:[response bytes] offset:3];
        size_t mtu_size = usb_dfu_object_set_mtu(mtu);
        [[ToolLogFile defaultLogger] debugWithFormat:@"ToolDFUCommand: MTU=%d", mtu_size];
        return true;
    }

    - (bool)sendSelectObjectRequest:(uint8_t)objectType pMaxCreateSize:(size_t *)pMaxCreateSize {
        // SELECT OBJECT 06 xx C0 -> 60 06 xx 00 01 00 00 00 00 00 00 00 00 00 00 C0
        uint8_t request[] = {
            NRF_DFU_OP_OBJECT_SELECT, objectType,
            NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
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
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
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
            if ([self sendRequestData:frame] == false) {
                return false;
            }
#if CDC_ACM_LOG_DEBUG
            // ログ出力
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"CDC ACM Send (%d bytes)", [frame length]];
#endif
        }
        return true;
    }

    - (bool)sendGetCrcRequest:(uint8_t)objectType imageSize:(size_t)imageSize {
        // CRC GET 03 C0 -> 60 03 01 87 00 00 00 38 f4 97 72 C0
        uint8_t request[] = {NRF_DFU_OP_CRC_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスデータから、エスケープシーケンスを取り除く
        NSData *respUnesc = [self unescapeResponseData:response];

        // 送信データ長を検証
        size_t recvSize = (size_t)[self convertLEBytesToUint32:[respUnesc bytes] offset:3];
        if (recvSize != imageSize) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"ToolDFUCommand: send object %d failed (expected %d bytes, recv %d bytes)",
             objectType, imageSize, recvSize];
            return false;
        }
        // チェックサムを検証
        uint32_t checksum = [self convertLEBytesToUint32:[respUnesc bytes] offset:7];
        if (checksum != usb_dfu_object_checksum_get()) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"ToolDFUCommand: send object %d failed (checksum error)",
             objectType];
            return false;
        }
        return true;
    }

    - (bool)sendExecuteObjectRequest {
        // EXECUTE OBJECT 04 C0 -> 60 04 01 C0
        static uint8_t request[] = {NRF_DFU_OP_OBJECT_EXECUTE, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
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

    - (uint16_t)convertLEBytesToUint16:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
        return uint;
    }

    - (uint32_t)convertLEBytesToUint32:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint32_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8)
            | ((uint32_t)bytes[offset + 2] << 16) | ((uint32_t)bytes[offset + 3] << 24);
        return uint;
    }

    - (void)convertUint32ToLEBytes:(uint32_t)uint data:(uint8_t *)data offset:(uint16_t)offset {
        uint8_t *bytes = data + offset;
        for (uint8_t i = 0; i < 4; i++) {
            *bytes++ = uint & 0xff;
            uint = uint >> 8;
        }
    }

    - (NSData *)sendRequest:(NSData *)data timeoutSec:(double)timeout {
#if CDC_ACM_LOG_DEBUG
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Send (%d bytes):", [data length]];
        [[ToolLogFile defaultLogger] hexdump:data];
#endif
        // データ送信
        if ([self sendRequestData:data] == false) {
            return nil;
        }
        // データを受信
        NSData *dataRecv = [[self toolCDCHelper] readFromDevice:timeout];
        if (dataRecv == nil) {
            return nil;
        }
#if CDC_ACM_LOG_DEBUG
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Recv (%d bytes):", [dataRecv length]];
        [[ToolLogFile defaultLogger] hexdump:dataRecv];
#endif
        return dataRecv;
    }

    - (bool)sendRequestData:(NSData *)data {
        // データを送信
        return [[self toolCDCHelper] writeToDevice:data];
    }

    - (bool)assertDFUResponseSuccess:(NSData *)response {
        // レスポンスを検証
        if (response == nil) {
            return false;
        }
        // ステータスコードを参照し、処理が成功したかどうかを判定
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        return (responseBytes[2] == NRF_DFU_BYTE_RESP_SUCCESS);
    }

    - (NSString *)getConnectedDevicePath {
        // 接続されているUSB CDC ACMデバイスのパスを走査
        NSArray *ACMDevicePathList = [[self toolCDCHelper] createACMDevicePathList];
        if (ACMDevicePathList != nil) {
            uint8_t id = 0xac;
            for (NSString *ACMDevicePath in ACMDevicePathList) {
                // 接続を実行
                if ([[self toolCDCHelper] connectDeviceTo:ACMDevicePath] == false) {
                    return nil;
                }
                // DFU PINGを実行
                if ([self sendPingRequest:id]) {
                    // 成功した場合は、接続された状態で、デバイスのパスを戻す
                    [[ToolLogFile defaultLogger]
                     infoWithFormat:@"DFU target device found: %@", ACMDevicePath];
                    return ACMDevicePath;
                } else {
                    // 失敗した場合は、接続を閉じ、次のデバイスに移る
                    [[self toolCDCHelper] disconnectDevice];
                }
            }
        }
        // デバイスが未接続の場合は、NULLを戻す
        [[ToolLogFile defaultLogger] error:@"DFU target device not found"];
        return nil;
    }

@end
