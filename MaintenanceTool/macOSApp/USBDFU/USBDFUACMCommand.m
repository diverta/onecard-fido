//
//  USBDFUACMCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/20.
//
#import <IOKit/serial/IOSerialKeys.h>

#import "debug_log.h"
#import "usb_cdc_util.h"

#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "USBDFUACMCommand.h"
#import "USBDFUDefine.h"

@interface USBDFUACMCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // 接続中のデバイスパスを保持
    @property (nonatomic) NSString                     *openingDevicePath;

@end

@implementation USBDFUACMCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
        }
        return self;
    }

    - (void)establishACMConnection {
        // DFU対象デバイスに対し、USB CDC ACM接続を実行（最大５秒間繰り返す）
        NSString *ACMDevicePath = nil;
        for (int i = 0; i < MAX_CNT_FOR_ACM_CONNECT; i++) {
            // １秒間ウェイト
            [NSThread sleepForTimeInterval:INTERVAL_SEC_FOR_ACM_CONNECT];
            // DFU対象デバイスに接続
            ACMDevicePath = [self getConnectedDevicePath];
            if (ACMDevicePath != nil) {
                break;
            }
        }
        if (ACMDevicePath == nil) {
            // 接続デバイスが見つからなかった場合
            [[ToolLogFile defaultLogger] error:@"DFU target device not found"];
            [[self delegate] didEstablishACMConnection:false];
        } else {
            // 接続デバイスが見つかった場合
            [[ToolLogFile defaultLogger] infoWithFormat:@"DFU target device found: %@", ACMDevicePath];
            [[self delegate] didEstablishACMConnection:true];
        }
    }

    - (void)closeACMConnection {
        [self disconnectDevice];
    }

#pragma mark - private functions

    - (NSString *)getConnectedDevicePath {
        // 接続されているUSB CDC ACMデバイスのパスを走査
        NSArray *ACMDevicePathList = [self createACMDevicePathList];
        if (ACMDevicePathList != nil) {
            uint8_t id = 0xac;
            for (NSString *ACMDevicePath in ACMDevicePathList) {
                // 接続を実行
                if ([self connectDeviceTo:ACMDevicePath] == false) {
                    return nil;
                }
                // DFU PINGを実行
                if ([self sendPingRequest:id]) {
                    // 成功した場合は、接続された状態で、デバイスのパスを戻す
                    return ACMDevicePath;
                } else {
                    // 失敗した場合は、接続を閉じ、次のデバイスに移る
                    [self disconnectDevice];
                }
            }
        }
        // デバイスが未接続の場合は、NULLを戻す
        return nil;
     }

    - (NSArray *)createACMDevicePathList {
        // シリアルデバイスのインスタンスを走査
        CFMutableDictionaryRef classesToMatch = IOServiceMatching(kIOSerialBSDServiceValue);
        if (classesToMatch == NULL) {
            [[ToolLogFile defaultLogger] error:@"ToolCDCHelper: IOServiceMatching returned NULL"];
            return nil;
        }
        
        // シリアルデバイスの情報を取得
        CFDictionarySetValue(classesToMatch, CFSTR(kIOSerialBSDTypeKey), CFSTR(kIOSerialBSDAllTypes));
        io_iterator_t serviceIterator = 0;
        kern_return_t kernResult =
        IOServiceGetMatchingServices(kIOMasterPortDefault, classesToMatch, &serviceIterator);
        if (kernResult != KERN_SUCCESS) {
            [[ToolLogFile defaultLogger] errorWithFormat:
             @"ToolCDCHelper: IOServiceGetMatchingServices returned %d", kernResult];
            return nil;
        }
        if (serviceIterator == 0) {
            [[ToolLogFile defaultLogger] error:@"ToolCDCHelper: serviceIterator not exist"];
            return nil;
        }
        
        // ACMシリアルデバイスのパス文字列を取得（複数件になるためリストに格納）
        NSArray *deviceList = [self generateACMDevicePathListFrom:serviceIterator];
        (void)IOObjectRelease(serviceIterator);
        return deviceList;
    }

    - (NSArray *)generateACMDevicePathListFrom:(io_iterator_t)serviceIterator {
        // 空のリストを作成
        NSMutableArray *deviceList = [NSMutableArray array];
        // 接続されているシリアルデバイスを走査
        io_object_t serialService;
        while ((serialService = IOIteratorNext(serviceIterator)) != 0) {
            NSString *serialTypeString = [self getSerialTypeString:serialService];
            if (serialTypeString) {
                if ([serialTypeString isEqualToString:@"usbmodem"]) {
                    NSString *devicePathString = [self getDevicePathString:serialService];
                    if (devicePathString) {
                        // USB CDC ACMデバイスである場合、そのパスをリストに格納
                        [deviceList addObject:devicePathString];
                    }
                }
            }
            (void)IOObjectRelease(serialService);
        }
        // USB CDC ACMデバイスのパスが格納されたリストを戻す
        return deviceList;
    }

    - (NSString *)getSerialTypeString:(io_object_t)serialService {
        NSString *serialTypeString = nil;
        CFStringRef serialType = (CFStringRef)IORegistryEntryCreateCFProperty(serialService, CFSTR(kIOTTYBaseNameKey), kCFAllocatorDefault, 0);
        if (serialType) {
            serialTypeString = [[NSString alloc] initWithString:(__bridge NSString *)serialType];
            CFRelease(serialType);
        }
        return serialTypeString;
    }

    - (NSString *)getDevicePathString:(io_object_t)serialService {
        NSString *devicePathString = nil;
        CFStringRef devicePath = (CFStringRef)IORegistryEntryCreateCFProperty(serialService, CFSTR(kIOCalloutDeviceKey), kCFAllocatorDefault, 0);
        if (devicePath) {
            devicePathString = [[NSString alloc] initWithString:(__bridge NSString *)devicePath];
            CFRelease(devicePath);
        }
        return devicePathString;
    }

    - (bool)connectDeviceTo:(NSString *)ACMDevicePath {
        [self setOpeningDevicePath:nil];
        
        // デバイスの正式パスを取得
        const char *path = [ACMDevicePath fileSystemRepresentation];
        if (path == NULL) {
            return false;
        }
        
        // デバイスがビジー状態でないかどうか検査
        if (usb_cdc_acm_device_is_not_busy(path) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolCDCHelper: %s", log_debug_message()];
            return false;
        }

        // デバイス接続を実行
        if (usb_cdc_acm_device_open(path) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolCDCHelper: %s", log_debug_message()];
            return false;
        }

        // 現在オープンされているデバイスのパスを保持
        [self setOpeningDevicePath:ACMDevicePath];
        [[ToolLogFile defaultLogger] debugWithFormat:@"ToolCDCHelper: %@ opened", [self openingDevicePath]];
        return true;
    }

    - (void)disconnectDevice {
        if ([self openingDevicePath]) {
            [[ToolLogFile defaultLogger] debugWithFormat:@"ToolCDCHelper: %@ closed", [self openingDevicePath]];
        }
        // デバイスから切断
        usb_cdc_acm_device_close();
    }

#pragma mark - Write & read function

    - (bool)writeToDevice:(NSData *)data {
        // 現在オープンされているデバイスへ、引数のバイト配列を書込み
        const char *dataBytes = (const char *)[data bytes];
        NSUInteger dataLength = [data length];
        if (usb_cdc_acm_device_write(dataBytes, dataLength) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolCDCHelper: %s", log_debug_message()];
            return false;
        }
        return true;
    }

    - (NSData *)readFromDevice:(double)timeout_sec {
        // 現在オープンされているデバイスから、内部バッファにバイト配列を読込
        if (usb_cdc_acm_device_read(timeout_sec) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"ToolCDCHelper: %s", log_debug_message()];
            return nil;
        }
        // 読込データをNSDataオブジェクトに変換
        NSData *readData = [[NSData alloc] initWithBytes:usb_cdc_acm_device_read_buffer() length:usb_cdc_acm_device_read_size()];
        return readData;
    }

#pragma mark - Send & receive utility

    - (NSData *)sendRequest:(NSData *)data timeoutSec:(double)timeout {
#if CDC_ACM_LOG_DEBUG
        [[ToolLogFile defaultLogger] debugWithFormat:@"CDC ACM Send (%d bytes):", [data length]];
        [[ToolLogFile defaultLogger] hexdump:data];
#endif
        // データ送信
        if ([self sendRequestData:data] == false) {
            return nil;
        }
        // データを受信
        NSData *dataRecv = [self readFromDevice:timeout];
        if (dataRecv == nil) {
            return nil;
        }
#if CDC_ACM_LOG_DEBUG
        [[ToolLogFile defaultLogger] debugWithFormat:@"CDC ACM Recv (%d bytes):", [dataRecv length]];
        [[ToolLogFile defaultLogger] hexdump:dataRecv];
#endif
        return dataRecv;
    }

    - (bool)sendRequestData:(NSData *)data {
        // データを送信
        return [self writeToDevice:data];
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

#pragma mark - DFU ping process

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

@end
