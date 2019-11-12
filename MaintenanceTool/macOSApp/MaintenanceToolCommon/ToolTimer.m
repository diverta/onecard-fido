//
//  ToolTimer.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/04/17.
//
#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "ToolTimer.h"

@interface ToolTimer ()

@end

@implementation ToolTimer

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id<ToolTimerDelegate>)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

    - (void)startTimeoutMonitorForSelector:(SEL)selector withObject:object afterDelay:(NSTimeInterval)delay {
        [self cancelTimeoutMonitorForSelector:selector withObject:object];
        [self performSelector:selector withObject:object afterDelay:delay];
    }

    - (void)cancelTimeoutMonitorForSelector:(SEL)selector withObject:object {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:selector object:object];
    }

#pragma mark - Scanning Timeout Monitor

    - (void)startScanningTimeoutMonitor {
        // スキャンタイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)
                                  withObject:nil afterDelay:10.0];
    }

    - (void)cancelScanningTimeoutMonitor {
        // スキャンタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)
                                   withObject:nil];
    }

    - (void)scanningTimeoutMonitorDidTimeout {
        // スキャンタイムアウト時の処理を実行
        [[self delegate] scanningDidTimeout];
    }

#pragma mark - Connecting Timeout Monitor

    - (void)startConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(connectionTimeoutMonitorDidTimeout)
                                  withObject:peripheral afterDelay:10.0];
    }

    - (void)cancelConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(connectionTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
    }

    - (void)connectionTimeoutMonitorDidTimeout {
        // 接続タイムアウト時の処理を実行
        [[self delegate] connectionDidTimeout];
    }

#pragma mark - Response Timeout Monitor

    - (void)startResponseTimeoutMonitor:(CBCharacteristic *)characteristic {
        // レスポンスタイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                  withObject:characteristic afterDelay:10.0];
    }

    - (void)cancelResponseTimeoutMonitor:(CBCharacteristic *)characteristic {
        // レスポンスタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(responseTimeoutMonitorDidTimeout)
                                   withObject:characteristic];
    }

    - (void)responseTimeoutMonitorDidTimeout {
        // レスポンスタイムアウト時の処理を実行
        [[self delegate] responseDidTimeout];
    }

#pragma mark - Discover Services Timeout Monitor

    - (void)startDiscoverServicesTimeoutMonitor:(CBPeripheral *)peripheral {
        // サービスディスカバータイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(discoverServicesTimeoutMonitorDidTimeout)
                                  withObject:peripheral afterDelay:10.0];
    }

    - (void)cancelDiscoverServicesTimeoutMonitor:(CBPeripheral *)peripheral {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverServicesTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
    }

    - (void)discoverServicesTimeoutMonitorDidTimeout {
        // サービスディスカバータイムアウト時の処理を実行
        [[self delegate] discoverServicesDidTimeout];
    }

#pragma mark - Discover Characteristics Timeout Monitor

    - (void)startDiscoverCharacteristicsTimeoutMonitor:(CBService *)service {
        // サービスディスカバータイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(discoverCharacteristicsTimeoutMonitorDidTimeout)
                                  withObject:service afterDelay:10.0];
    }

    - (void)cancelDiscoverCharacteristicsTimeoutMonitor:(CBService *)service {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverCharacteristicsTimeoutMonitorDidTimeout)
                                   withObject:service];
    }

    - (void)discoverCharacteristicsTimeoutMonitorDidTimeout {
        // キャラクタリスティック・ディスカバーのタイムアウト時の処理を実行
        [[self delegate] discoverCharacteristicsDidTimeout];
    }

#pragma mark - Subscribe Characteristic Timeout Monitor

    - (void)startSubscribeCharacteristicTimeoutMonitor:(CBCharacteristic *)characteristic {
        // 監視ステータス更新タイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(subscribeCharacteristicTimeoutMonitorDidTimeout)
                                  withObject:characteristic afterDelay:10.0];
    }

    - (void)cancelSubscribeCharacteristicTimeoutMonitor:(CBCharacteristic *)characteristic {
        // 監視ステータス更新タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(subscribeCharacteristicTimeoutMonitorDidTimeout)
                                   withObject:characteristic];
    }

    - (void)subscribeCharacteristicTimeoutMonitorDidTimeout {
        // 監視ステータス更新タイムアウト時の処理を実行
        [[self delegate] subscribeCharacteristicDidTimeout];
    }

@end
