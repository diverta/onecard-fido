//
//  ToolTimer.m
//  U2FMaintenanceTool
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
        NSLog(@"ScanningTimeoutMonitor started");
    }

    - (void)cancelScanningTimeoutMonitor {
        // スキャンタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)
                                   withObject:nil];
        NSLog(@"ScanningTimeoutMonitor canceled");
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
        NSLog(@"ConnectionTimeoutMonitor started");
    }

    - (void)cancelConnectionTimeoutMonitor:(CBPeripheral *)peripheral {
        // 接続タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(connectionTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
        NSLog(@"ConnectionTimeoutMonitor canceled");
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
        NSLog(@"DiscoverServicesTimeoutMonitor started");
    }

    - (void)cancelDiscoverServicesTimeoutMonitor:(CBPeripheral *)peripheral {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverServicesTimeoutMonitorDidTimeout)
                                   withObject:peripheral];
        NSLog(@"DiscoverServicesTimeoutMonitor canceled");
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
        NSLog(@"DiscoverCharacteristicsTimeoutMonitor started");
    }

    - (void)cancelDiscoverCharacteristicsTimeoutMonitor:(CBService *)service {
        // サービスディスカバータイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(discoverCharacteristicsTimeoutMonitorDidTimeout)
                                   withObject:service];
        NSLog(@"DiscoverCharacteristicsTimeoutMonitor canceled");
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
        NSLog(@"SubscribeCharacteristicTimeoutMonitor started");
    }

    - (void)cancelSubscribeCharacteristicTimeoutMonitor:(CBCharacteristic *)characteristic {
        // 監視ステータス更新タイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(subscribeCharacteristicTimeoutMonitorDidTimeout)
                                   withObject:characteristic];
        NSLog(@"SubscribeCharacteristicTimeoutMonitor canceled");
    }

    - (void)subscribeCharacteristicTimeoutMonitorDidTimeout {
        // 監視ステータス更新タイムアウト時の処理を実行
        [[self delegate] subscribeCharacteristicDidTimeout];
    }

@end
