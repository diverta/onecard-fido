//
//  ToolHIDHelper.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/19.
//
#import <Foundation/Foundation.h>
#import <IOKit/hid/IOHIDManager.h>

#import "ToolHIDHelper.h"
#import "ToolCommon.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

#define HID_PACKET_SIZE       64
#define HID_INIT_HEADER_SIZE  7
#define HID_CONT_HEADER_SIZE  5
#define HID_INIT_PAYLOAD_SIZE (HID_PACKET_SIZE - HID_INIT_HEADER_SIZE)
#define HID_CONT_PAYLOAD_SIZE (HID_PACKET_SIZE - HID_CONT_HEADER_SIZE)

@interface ToolHIDHelper ()
    @property (nonatomic) IOHIDManagerRef   toolHIDManager;
    @property (nonatomic) IOHIDDeviceRef    toolHIDDevice;
    @property (nonatomic) NSMutableData    *hidResponse;

    // リクエスト送信時のCIDを保持
    @property (nonatomic) NSData           *requestCID;
@end

@implementation ToolHIDHelper

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolHIDHelperDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
            [self initializeHIDManager];
        }
        return self;
    }

    - (void)initializeHIDManager {
        // IOHIDManagerをデフォルトの設定で作成
        [self setToolHIDManager:IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDManagerOptionNone)];
        // マッチングするデバイスの条件を設定
        NSDictionary* criteria = @{
            @kIOHIDDeviceUsagePageKey: @(0xf1d0),
            @kIOHIDDeviceUsageKey: @(0x01),
            @kIOHIDVendorIDKey: @(0xf055),
            @kIOHIDProductIDKey: @(0x0001),
        };
        IOHIDManagerSetDeviceMatching(
            [self toolHIDManager], (__bridge CFDictionaryRef)criteria);
        IOHIDManagerScheduleWithRunLoop(
            [self toolHIDManager], CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOReturn ret = IOHIDManagerOpen(
            [self toolHIDManager], kIOHIDOptionsTypeNone);
        if (ret != kIOReturnSuccess) {
            [[ToolLogFile defaultLogger] error:MSG_USB_DETECT_FAILED];
            return;
        }
        // ハンドラー定義
        IOHIDManagerRegisterDeviceMatchingCallback(
            [self toolHIDManager], &handleDeviceMatching, (__bridge void *)self);
        IOHIDManagerRegisterDeviceRemovalCallback(
            [self toolHIDManager], &handleDeviceRemoval, (__bridge void *)self);
        IOHIDManagerRegisterInputReportCallback(
            [self toolHIDManager], &handleInputReport, (__bridge void *)self);
        [[ToolLogFile defaultLogger] info:MSG_USB_DETECT_STARTED];
    }

    - (bool)isDeviceConnected {
        if ([self toolHIDDevice] == nil) {
            return false;
        }
        return true;
    }

#pragma mark - Response timeout monitor

    - (void)startTimeoutMonitorForSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay {
        [self cancelTimeoutMonitorForSelector:selector withObject:object];
        [self performSelector:selector withObject:object afterDelay:delay];
    }

    - (void)cancelTimeoutMonitorForSelector:(SEL)selector withObject:object {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:selector object:object];
    }

    - (void)startResponseTimeoutMonitor {
        // タイムアウト監視を開始（30秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                  withObject:nil afterDelay:30.0];
    }

    - (void)cancelResponseTimeoutMonitor {
        // タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                   withObject:nil];
        // リクエスト送信時のCIDをクリア
        [self setRequestCID:nil];
    }

    - (void)responseTimeoutMonitorDidTimeout {
        // タイムアウト時は呼出元に制御を戻す
        [[ToolLogFile defaultLogger] error:MSG_HID_CMD_RESPONSE_TIMEOUT];
        [[self delegate] hidHelperDidResponseTimeout];
        // リクエスト送信時のCIDをクリア
        [self setRequestCID:nil];
    }

#pragma mark - Functions for receive messages from HID device

    - (void)HIDManagerDidReceiveMessage:(uint8_t *)message length:(long)length {
        static uint16_t remaining;
        uint16_t        datalen;
        uint16_t        dump_data_len;
        static uint8_t  receivedCmd;
        
        NSData *reportData = [[NSData alloc] initWithBytes:message length:length];

        // CIDは先頭から４バイトを取得
        NSData *cid = [reportData subdataWithRange:NSMakeRange(0, 4)];
        // レスポンス受信時のCIDが、リクエスト送信時のCIDと異なる場合は無視
        if ([cid isEqualToData:[self requestCID]] == false) {
            return;
        }
        // コマンド／シーケンスは先頭から５バイト目を参照
        uint8_t cmd = message[4];
        if (cmd & 0x80) {
            // データ格納領域を初期化
            [self setHidResponse:[NSMutableData alloc]];
            // INITフレームから、８バイト目以降のデータを連結（最大57バイト）
            remaining = ((message[5] << 8) & 0xff00) | (message[6] & 0x00ff);
            datalen = (remaining < HID_INIT_PAYLOAD_SIZE) ? remaining : HID_INIT_PAYLOAD_SIZE;
            [[self hidResponse]
             appendData:[reportData subdataWithRange:NSMakeRange(HID_INIT_HEADER_SIZE, datalen)]];
            // コマンドを退避
            receivedCmd = cmd;
            // ログ出力
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"HID Recv INIT frame: data size=%d length=%d",
             remaining, datalen];
            dump_data_len = datalen + HID_INIT_HEADER_SIZE;

        } else {
            // CONTフレームから、６バイト目以降のデータを連結（最大59バイト）
            datalen = (remaining < HID_CONT_PAYLOAD_SIZE) ? remaining : HID_CONT_PAYLOAD_SIZE;
            [[self hidResponse]
             appendData:[reportData subdataWithRange:NSMakeRange(HID_CONT_HEADER_SIZE, datalen)]];
            // ログ出力
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"HID Recv CONT frame: seq=%d length=%d", cmd, datalen];
            dump_data_len = datalen + HID_CONT_HEADER_SIZE;
        }
        // フレーム内容をログ出力
        [[ToolLogFile defaultLogger]
         hexdump:[reportData subdataWithRange:NSMakeRange(0, dump_data_len)]];
        // パケットをすべて受信したら、データをアプリケーションに引き渡す
        remaining -= datalen;
        if (remaining == 0) {
            if (receivedCmd != 0xbb) {
                // レスポンスタイムアウト監視を停止
                [self cancelResponseTimeoutMonitor];
                // キープアライブレスポンス以外であれば、情報をコンソール出力し、
                // アプリケーションに制御を戻す
                [[self delegate] hidHelperDidReceive:[self hidResponse] CID:cid CMD:receivedCmd];
            }
        }
    }

#pragma mark - Functions for send message to HID device

    - (void)hidHelperWillSend:(NSData *)message
                          CID:(NSData *)cid CMD:(uint8_t)command {
        // レスポンスタイムアウトを監視
        [self startResponseTimeoutMonitor];
        // リクエスト送信時のCIDを保持
        [self setRequestCID:cid];
        // 送信フレームを生成し、各フレームをHIDデバイスに送信
        NSArray<NSData *> *requestFrames =
        [self generateHIDRequestFramesFrom:message CID:cid CMD:command];
        [self HIDManagerWillSendRequestFrames:requestFrames];
    }

    - (void)setCIDBytes:(char *)data_buf CID:(NSData *)cid {
        char *cidBytes = (char *)[cid bytes];
        for (uint8_t j = 0; j < 4; j++) {
            data_buf[j] = cidBytes[j];
        }
    }

    - (NSArray<NSData *> *)generateHeaderFrameFrom:(NSData *)message
                                               CID:(NSData *)cid CMD:(uint8_t)cmd {
        // 作業領域を初期化
        NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
        char xfer_data[HID_INIT_HEADER_SIZE];
        memset(xfer_data, 0x00, sizeof(xfer_data));
        // ヘッダー（CID、CMD、データ長）だけのフレームを生成
        [self setCIDBytes:xfer_data CID:cid];
        xfer_data[4] = cmd;
        NSData *xferMessage = [[NSData alloc] initWithBytes:xfer_data length:HID_INIT_HEADER_SIZE];
        // ログ出力
        [[ToolLogFile defaultLogger]
         debug:@"HID Sent INIT frame: data size=0 length=0"];
        [[ToolLogFile defaultLogger]
         hexdump:[xferMessage subdataWithRange:NSMakeRange(0, HID_INIT_HEADER_SIZE)]];
        [array addObject:xferMessage];
        return array;
    }

    - (NSArray<NSData *> *)generateHIDRequestFramesFrom:(NSData *)message
                                                    CID:(NSData *)cid CMD:(uint8_t)command {
        // メッセージをバイト配列に変換
        char    *dataBytes = (char *)[message bytes];
        uint16_t dataLength = (uint16_t)[message length];
        if (dataLength == 0) {
            // データ長が0の場合は、ヘッダー（CID、CMD、データ長）だけのフレームを生成
            return [self generateHeaderFrameFrom:message CID:cid CMD:command];
        }
        // 分割されたメッセージを格納する配列
        NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
        // 分割送信
        char     xfer_data[HID_PACKET_SIZE];
        uint16_t xfer_data_max;
        uint16_t xfer_data_len;
        uint16_t dump_data_len;
        uint16_t remaining;
        uint16_t seq = 0;
        for (uint16_t i = 0; i < dataLength; i += xfer_data_len) {
            // データ長（INIT=57バイト、CONT=59バイト）
            remaining = dataLength - i;
            xfer_data_max = (i == 0) ? HID_INIT_PAYLOAD_SIZE : HID_CONT_PAYLOAD_SIZE;
            xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;
            // 送信パケットを編集
            memset(xfer_data, 0x00, sizeof(xfer_data));
            [self setCIDBytes:xfer_data CID:cid];
            if (i == 0) {
                xfer_data[4] = command;
                xfer_data[5] = (dataLength >> 8) & 0x00ff; // MSB(messageLength)
                xfer_data[6] = dataLength & 0x00ff;        // LSB(messageLength)
                memcpy(xfer_data + HID_INIT_HEADER_SIZE, dataBytes + i, xfer_data_len);
                // ログ出力
                [[ToolLogFile defaultLogger]
                 debugWithFormat:@"HID Sent INIT frame: data size=%d length=%d",
                 dataLength, xfer_data_len];
                dump_data_len = xfer_data_len + HID_INIT_HEADER_SIZE;
            } else {
                xfer_data[4] = seq; // SEQ
                memcpy(xfer_data + HID_CONT_HEADER_SIZE, dataBytes + i, xfer_data_len);
                // ログ出力
                [[ToolLogFile defaultLogger]
                 debugWithFormat:@"HID Sent CONT frame: seq=%d length=%d",
                 seq++, xfer_data_len];
                dump_data_len = xfer_data_len + HID_CONT_HEADER_SIZE;
            }
            NSData *xferMessage = [[NSData alloc] initWithBytes:xfer_data length:sizeof(xfer_data)];
            // フレーム内容をログ出力
            [[ToolLogFile defaultLogger]
             hexdump:[xferMessage subdataWithRange:NSMakeRange(0, dump_data_len)]];
            [array addObject:xferMessage];
        }
        return array;
    }

    - (void)HIDManagerWillSendRequestFrames:(NSArray<NSData *> *)frames {
        // レスポンスの各フレームをHIDデバイスに送信
        for (NSData *frame in frames) {
            uint8_t *reportBytes = (uint8_t *)[frame bytes];
            CFIndex  reportLength = [frame length];
            IOReturn ret = IOHIDDeviceSetReport([self toolHIDDevice], kIOHIDReportTypeOutput, 0x00,
                                                reportBytes, reportLength);
            // 送信失敗時は情報をログ出力
            if (ret != kIOReturnSuccess) {
                [[ToolLogFile defaultLogger] error:@"ToolHIDHelper send failed"];
            }
        }
    }

#pragma mark - Non Objective-C codes

    void handleDeviceMatching(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
        // HIDデバイスの参照を保持
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf setToolHIDDevice:device];
        // コマンドクラスに通知
        [[helperSelf delegate] hidHelperDidDetectConnect];
    }
    void handleDeviceRemoval(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
        // HIDデバイス参照を解除
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf setToolHIDDevice:nil];
        // コマンドクラスに通知
        [[helperSelf delegate] hidHelperDidDetectRemoval];
    }
    void handleInputReport(void *context, IOReturn result, void *sender, IOHIDReportType type,
                           uint32_t reportID, uint8_t *report, CFIndex reportLength) {
        // 受信メッセージを転送
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf HIDManagerDidReceiveMessage:report length:reportLength];
    }

@end
