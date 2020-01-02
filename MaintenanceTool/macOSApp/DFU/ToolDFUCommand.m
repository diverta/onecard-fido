//
//  ToolDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#import <Foundation/Foundation.h>

#import "ToolCDCHelper.h"
#import "ToolDFUCommand.h"
#import "ToolLogFile.h"

@interface ToolDFUCommand ()

    @property (nonatomic) ToolCDCHelper *toolCDCHelper;

    // デバイスから取得したMTUを保持
    @property (nonatomic) uint16_t MTU;

@end

@implementation ToolDFUCommand

    - (id)init {
        self = [super init];
        
        // ToolCDCHelperのインスタンスを生成
        [self setToolCDCHelper:[[ToolCDCHelper alloc] init]];
        return self;
    }

    - (NSString *)testMain {
        NSString *ACMDevicePath = [self getConnectedDevicePath];
        if (ACMDevicePath == nil) {
            return @"DFU対象デバイスが接続されていません.";
        }
        // DFU対象デバイスからMTUを取得
        if ([self sendSetReceiptRequest]) {
            [self sendGetMtuRequest];
        }
        // 処理完了
        [[self toolCDCHelper] disconnectDevice];
        return ACMDevicePath;
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

    - (uint16_t)convertLEBytesToUint16:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
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
