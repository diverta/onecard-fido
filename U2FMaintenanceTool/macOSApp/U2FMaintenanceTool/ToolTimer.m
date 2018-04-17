//
//  ToolTimer.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/04/17.
//
#import <Foundation/Foundation.h>
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

    - (void)startTimeoutMonitorForSelector:(SEL)selector afterDelay:(NSTimeInterval)delay {
        [self cancelTimeoutMonitorForSelector:selector];
        [self performSelector:selector withObject:nil afterDelay:delay];
    }

    - (void)cancelTimeoutMonitorForSelector:(SEL)selector {
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:selector object:nil];
    }

#pragma mark - Scanning Timeout Monitor

    - (void)startScanningTimeoutMonitor {
        // スキャンタイムアウト監視を開始（10秒後にタイムアウト）
        [self startTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout) afterDelay:10.0];
        NSLog(@"ScanningTimeoutMonitor started");
    }

    - (void)cancelScanningTimeoutMonitor {
        // スキャンタイムアウト監視を停止
        [self cancelTimeoutMonitorForSelector:@selector(scanningTimeoutMonitorDidTimeout)];
        NSLog(@"ScanningTimeoutMonitor canceled");
    }

    - (void)scanningTimeoutMonitorDidTimeout {
        // スキャンタイムアウト時の処理を実行
        [[self delegate] scanningDidTimeout];
    }

@end
