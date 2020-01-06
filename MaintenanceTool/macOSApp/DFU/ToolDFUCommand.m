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

@interface ToolDFUCommand ()

    @property (nonatomic) ToolCDCHelper *toolCDCHelper;

    // デバイスから取得した最大送信可能サイズを保持
    @property (nonatomic) uint16_t MTU;
    @property (nonatomic) uint32_t maxDatSize;

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
        if ([self sendSelectDatObjectRequest] == false) {
            return false;
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

    - (bool)sendPingRequest {
        // PING 09 01 C0 -> 60 09 01 01 C0
        static uint8_t pingRequest[] = {0x09, 0x01, 0xc0};
        NSData *data = [NSData dataWithBytes:pingRequest length:sizeof(pingRequest)];
        NSData *response = [self sendRequest:data];
        if (response == nil) {
            return false;
        }
        // レスポンスを検証
        uint8_t *pingResponse = (uint8_t *)[response bytes];
        return (pingRequest[1] == pingResponse[3]);
    }

    - (bool)sendSetReceiptRequest {
        // SET RECEIPT 02 00 00 C0 -> 60 02 01 C0
        static uint8_t request[] = {0x02, 0x00, 0x00, 0xc0};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        return ([self sendRequest:data] != nil);
    }

    - (bool)sendGetMtuRequest {
        // Get the preferred MTU size on the request.
        // GET MTU 07 C0 -> 60 07 01 83 00 C0
        static uint8_t mtuRequest[] = {0x07, 0xc0};
        NSData *data = [NSData dataWithBytes:mtuRequest length:sizeof(mtuRequest)];
        NSData *response = [self sendRequest:data];
        if (response == nil) {
            return false;
        }
        // レスポンスからMTUを取得（4〜5バイト目）
        [self setMTU:[self convertLEBytesToUint16:[response bytes] offset:3]];
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: MTU=%d", [self MTU]];

        return true;
    }

    - (bool)sendSelectDatObjectRequest {
        // SELECT OBJECT (dat) を実行し、datイメージの最大送信可能サイズを取得
        uint32_t size = [self sendSelectObjectRequest:0x01];
        if (size == 0 || size < nrf52_app_image_dat_size()) {
            return false;
        }

        // 内部変数に保持
        [self setMaxDatSize:size];
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: max dat image size=%d", [self maxDatSize]];
        return true;
    }

    - (uint32_t)sendSelectObjectRequest:(uint8_t)object_number {
        // SELECT OBJECT 06 xx C0 -> 60 06 xx 00 01 00 00 00 00 00 00 00 00 00 00 C0
        uint8_t request[] = {0x06, object_number, 0xc0};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data];
        if (response == nil) {
            return 0;
        }
        // レスポンスから、イメージの最大送信可能サイズを取得（4〜7バイト目）
        return [self convertLEBytesToUint32:[response bytes] offset:3];
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

    - (NSData *)sendRequest:(NSData *)data {
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Send (%d bytes):", [data length]];
        [[ToolLogFile defaultLogger] hexdump:data];
        // データを送信
        if ([[self toolCDCHelper] writeToDevice:data] == false) {
            return nil;
        }
        // データを受信
        NSData *dataRecv = [[self toolCDCHelper] readFromDevice];
        if (dataRecv == nil) {
            return nil;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Recv (%d bytes):", [dataRecv length]];
        [[ToolLogFile defaultLogger] hexdump:dataRecv];
        return dataRecv;
    }

    - (NSString *)getConnectedDevicePath {
        // 接続されているUSB CDC ACMデバイスのパスを走査
        NSArray *ACMDevicePathList = [[self toolCDCHelper] createACMDevicePathList];
        if (ACMDevicePathList != nil) {
            for (NSString *ACMDevicePath in ACMDevicePathList) {
                // 接続を実行
                if ([[self toolCDCHelper] connectDeviceTo:ACMDevicePath] == false) {
                    return nil;
                }
                // DFU PINGを実行
                if ([self sendPingRequest]) {
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
