//
//  ToolHIDHelper.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/19.
//
#import <Foundation/Foundation.h>
#import <IOKit/hid/IOHIDManager.h>

#import "ToolHIDHelper.h"
#import "ToolCommon.h"

#define HID_PACKET_SIZE       64
#define HID_INIT_HEADER_SIZE  7
#define HID_CONT_HEADER_SIZE  5
#define HID_INIT_PAYLOAD_SIZE (HID_PACKET_SIZE - HID_INIT_HEADER_SIZE)
#define HID_CONT_PAYLOAD_SIZE (HID_PACKET_SIZE - HID_CONT_HEADER_SIZE)

@interface ToolHIDHelper ()
    @property (nonatomic) IOHIDManagerRef   toolHIDManager;
    @property (nonatomic) IOHIDDeviceRef    toolHIDDevice;
    @property (nonatomic) NSMutableData    *hidResponse;
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

    - (bool)isDeviceConnected {
        if ([self toolHIDDevice] == nil) {
            return false;
        }
        return true;
    }

#pragma mark - Functions for receive messages from HID device

    - (void)HIDManagerDidReceiveMessage:(uint8_t *)message length:(long)length {
        static uint16_t remaining;
        uint16_t        datalen;
        
        NSData *reportData = [[NSData alloc] initWithBytes:message length:length];
        NSLog(@"ToolHIDHelper receive: reportLength(%ld) report(%@)", length, reportData);
        
        // CIDは先頭から４バイトを取得
        NSData *cid = [reportData subdataWithRange:NSMakeRange(0, 4)];
        
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
            
        } else {
            // CONTフレームから、６バイト目以降のデータを連結（最大59バイト）
            datalen = (remaining < HID_CONT_PAYLOAD_SIZE) ? remaining : HID_CONT_PAYLOAD_SIZE;
            [[self hidResponse]
             appendData:[reportData subdataWithRange:NSMakeRange(HID_CONT_HEADER_SIZE, datalen)]];
        }
        // パケットをすべて受信したら、データをアプリケーションに引き渡す
        remaining -= datalen;
        if (remaining == 0) {
            NSLog(@"hidHelperDidReceive(CID=%@, CMD=%02x, %lu bytes): %@",
                  cid, cmd,
                  (unsigned long)[[self hidResponse] length],
                  [self hidResponse]);
            [[self delegate] hidHelperDidReceive:[self hidResponse] CID:cid CMD:cmd];
        }
    }

#pragma mark - Functions for send message to HID device

    - (void)hidHelperWillSend:(NSData *)message
                          CID:(NSData *)cid CMD:(uint8_t)command {
        NSLog(@"hidHelperWillSend(CID=%@, CMD=%02x, %lu bytes): %@",
              cid, command, (unsigned long)[message length], message);
        
        NSArray<NSData *> *requestFrames =
        [self generateHIDRequestFramesFrom:message CID:cid CMD:command];
        [self HIDManagerWillSendRequestFrames:requestFrames];
    }

    - (NSArray<NSData *> *)generateHIDRequestFramesFrom:(NSData *)message
                                                    CID:(NSData *)cid CMD:(uint8_t)command {
        // 分割されたメッセージを格納する配列
        NSMutableArray<NSData *> *array = [[NSMutableArray alloc] init];
        // メッセージをバイト配列に変換
        char    *dataBytes = (char *)[message bytes];
        uint16_t dataLength = (uint16_t)[message length];
        // 分割送信
        char     xfer_data[HID_PACKET_SIZE];
        uint16_t xfer_data_max;
        uint16_t xfer_data_len;
        uint16_t remaining;
        uint16_t seq = 0;
        for (uint16_t i = 0; i < dataLength; i += xfer_data_len) {
            // データ長（INIT=57バイト、CONT=59バイト）
            remaining = dataLength - i;
            xfer_data_max = (i == 0) ? HID_INIT_PAYLOAD_SIZE : HID_CONT_PAYLOAD_SIZE;
            xfer_data_len = (remaining < xfer_data_max) ? remaining : xfer_data_max;
            // 送信パケットを編集
            char *cidBytes = (char *)[cid bytes];
            for (uint8_t j = 0; j < 4; j++) {
                xfer_data[j] = cidBytes[j];
            }
            if (i == 0) {
                xfer_data[4] = command;
                xfer_data[5] = (dataLength >> 8) & 0x00ff; // MSB(messageLength)
                xfer_data[6] = dataLength & 0x00ff;        // LSB(messageLength)
                memcpy(xfer_data + HID_INIT_HEADER_SIZE, dataBytes + i, xfer_data_len);
            } else {
                xfer_data[4] = seq++; // SEQ
                memcpy(xfer_data + HID_CONT_HEADER_SIZE, dataBytes + i, xfer_data_len);
            }
            NSData *xferMessage = [[NSData alloc] initWithBytes:xfer_data length:sizeof(xfer_data)];
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
