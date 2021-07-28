//
//  MatterCommand.m
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/28.
//
#import <Foundation/Foundation.h>
#import "MatterCommand.h"

@interface MatterCommand ()

    // 画面の参照を保持
    @property (nonatomic, weak) id          viewControllerRef;
    // 非同期処理用のキュー（画面用）
    @property (nonatomic) dispatch_queue_t  mainQueue;

@end

@implementation MatterCommand

    - (id)init {
        return [self initWithViewControllerRef:nil];
    }

    - (id)initWithViewControllerRef:(id)ref {
        self = [super init];
        if (self) {
            // 画面の参照を保持
            [self setViewControllerRef:ref];
            // 画面スレッドにバインドされるデフォルトキューを取得
            [self setMainQueue:dispatch_get_main_queue()];
        }
        NSLog(@"initWithViewControllerRef done");
        return self;
    }

#pragma mark - for CHIPDevicePairingDelegate

    - (void)onPairingComplete:(NSError *)error {
        if (error.code != CHIPSuccess) {
            NSLog(@"Got pairing error back %@", error);
        } else {
            dispatch_async([self mainQueue], ^{
                // TODO: ペアリング完了後の処理を実行
            });
        }
    }

@end
