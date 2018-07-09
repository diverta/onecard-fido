//
//  ToolHIDHelper.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/07/03.
//
#import <Foundation/Foundation.h>
#import <IOKit/hid/IOHIDManager.h>

#import "ToolHIDHelper.h"
#import "ToolCommon.h"

@interface ToolHIDHelper ()

    @property (nonatomic) IOHIDManagerRef   toolHIDManager;
    @property (nonatomic) IOHIDDeviceRef    toolHIDDevice;
    @property (nonatomic) NSMutableData    *hidU2FRequest;

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
                                   @kIOHIDDeviceUsagePageKey: @(0xff00),
                                    @kIOHIDDeviceUsageKey: @(0x01),
                                    @kIOHIDVendorIDKey: @(0xf055),
                                    @kIOHIDProductIDKey: @(0x0001),
                                    };
        IOHIDManagerSetDeviceMatching([self toolHIDManager], (__bridge CFDictionaryRef)criteria);
        IOHIDManagerScheduleWithRunLoop([self toolHIDManager], CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOReturn ret = IOHIDManagerOpen([self toolHIDManager], kIOHIDOptionsTypeNone);
        if (ret != kIOReturnSuccess) {
            NSLog(@"initializeHIDManager: IOHIDManagerOpen failed");
            return;
        }
        // ハンドラー定義
        IOHIDManagerRegisterDeviceMatchingCallback(
            [self toolHIDManager], &handleDeviceMatching, (__bridge void *)self);
        IOHIDManagerRegisterDeviceRemovalCallback(
            [self toolHIDManager], &handleDeviceRemoval, (__bridge void *)self);
        IOHIDManagerRegisterInputReportCallback(
            [self toolHIDManager], &handleInputReport, (__bridge void *)self);
        NSLog(@"initializeHIDManager done");
    }

#pragma mark - Functions for receive messages from HID device

    - (void)HIDManagerDidReceiveMessage:(uint8_t *)message length:(long)length {
        static uint16_t remaining;
        uint16_t        datalen;

        // チャネルID（先頭４バイト）が0x00でない場合は無視
        for (int i = 0; i < 4; i++) {
            if (message[i] != 0x00) {
                return;
            }
        }
        NSData *reportData = [[NSData alloc] initWithBytes:message length:length];
        NSLog(@"ToolHIDHelper receive: reportLength(%ld) report(%@)", length, reportData);
        
        // コマンド／シーケンスは先頭から５バイト目を参照
        if (message[4] == 0x83) {
            // リクエストデータ格納領域を初期化
            [self setHidU2FRequest:[NSMutableData alloc]];
            // INITフレームから、８バイト目以降のデータを連結（最大25バイト）
            remaining = ((message[5] << 8) & 0xff00) | (message[6] & 0x00ff);
            datalen = (remaining < 25) ? remaining : 25;
            [[self hidU2FRequest] appendData:[reportData subdataWithRange:NSMakeRange(7, datalen)]];

        } else {
            // CONTフレームから、６バイト目以降のデータを連結（最大27バイト）
            datalen = (remaining < 27) ? remaining : 27;
            [[self hidU2FRequest] appendData:[reportData subdataWithRange:NSMakeRange(5, datalen)]];
        }
        // パケットをすべて受信したら、U2Fリクエスト（APDUヘッダー＋データ）をアプリケーションに引き渡す
        remaining -= datalen;
        if (remaining == 0) {
            NSLog(@"hidHelperDidReceive(%lu bytes): %@",
                  (unsigned long)[[self hidU2FRequest] length],
                  [self hidU2FRequest]);
            [[self delegate] hidHelperDidReceive:[self hidU2FRequest]];
        }
    }

#pragma mark - Functions for send message to HID device

    - (void)hidHelperWillSend:(NSData *)message {
        NSLog(@"hidHelperWillSend(%lu bytes): %@", (unsigned long)[message length], message);

        NSArray<NSData *> *responseFrames = [self generateU2FResponseFramesFrom:message];
        [self HIDManagerWillSendU2FResponseFrames:responseFrames];
    }

    - (NSArray<NSData *> *)generateU2FResponseFramesFrom:(NSData *)message {
        // 分割されたメッセージを格納する配列
        NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
        // メッセージをバイト配列に変換
        char    *apduBytes = (char *)[message bytes];
        uint16_t apduLength = (uint16_t)[message length];
        // 分割送信
        char     xfer_data[64];
        uint16_t xfer_data_max;
        uint16_t xfer_data_len;
        uint16_t remaining;
        uint16_t seq = 0;
        for (uint16_t i = 0; i < apduLength; i += xfer_data_len) {
            // データ長（INIT=57バイト、CONT=59バイト）
            remaining = apduLength - i;
            xfer_data_max = (i == 0) ? 57 : 59;
            xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;
            // 送信パケットを編集
            memset(xfer_data, 0x00, sizeof(xfer_data));
            if (i == 0) {
                xfer_data[4] = 0x83;  // CMD
                xfer_data[5] = (apduLength >> 8) & 0x00ff; // MSB(messageLength)
                xfer_data[6] = apduLength & 0x00ff;        // LSB(messageLength)
                memcpy(xfer_data + 7, apduBytes + i, xfer_data_len);
            } else {
                xfer_data[4] = seq++; // SEQ
                memcpy(xfer_data + 5, apduBytes + i, xfer_data_len);
            }
            NSData *xferMessage = [[NSData alloc] initWithBytes:xfer_data length:sizeof(xfer_data)];
            [array addObject:xferMessage];
        }
        return array;
    }

    - (void)HIDManagerWillSendU2FResponseFrames:(NSArray<NSData *> *)frames {
        // レスポンスの各フレームをHIDデバイスに送信
        for (NSData *frame in frames) {
            uint8_t *reportBytes = (uint8_t *)[frame bytes];
            CFIndex  reportLength = [frame length];
            IOReturn ret = IOHIDDeviceSetReport([self toolHIDDevice], kIOHIDReportTypeOutput, 0x00,
                                                reportBytes, reportLength);
            if (ret == kIOReturnSuccess) {
                NSLog(@"ToolHIDHelper send: messageLength(%ld) message(%@)",
                      (long)[frame length], frame);
            }
        }
    }

#pragma mark - Non Objective-C codes

    void handleDeviceMatching(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
        NSLog(@"ToolHIDHelper: HID Device detected.");
        // HIDデバイスの参照を保持
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf setToolHIDDevice:device];
    }
    void handleDeviceRemoval(void *context, IOReturn result, void *sender, IOHIDDeviceRef device) {
        NSLog(@"ToolHIDHelper: HID Device removed.");
        // HIDデバイス参照を解除
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf setToolHIDDevice:nil];
    }
    void handleInputReport(void *context, IOReturn result, void *sender, IOHIDReportType type,
                           uint32_t reportID, uint8_t *report, CFIndex reportLength) {
        // 受信メッセージを転送
        ToolHIDHelper *helperSelf = (__bridge ToolHIDHelper *)context;
        [helperSelf HIDManagerDidReceiveMessage:report length:reportLength];
    }

@end

