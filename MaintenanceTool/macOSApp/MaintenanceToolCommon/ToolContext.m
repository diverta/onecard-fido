//
//  ToolContext.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/07/16.
//
#import <Foundation/Foundation.h>
#import "ToolContext.h"

static ToolContext *sharedInstance;

@interface ToolContext ()
    // アプリケーションの参照を保持
    @property (nonatomic, weak) AppDelegate *delegate;

@end

@implementation ToolContext

#pragma mark - Methods for singleton

    + (ToolContext *)instance {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] init];
        });
        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Public methods

    - (void)setAppDelegateRef:(AppDelegate *)ref {
        // アプリケーションの参照を設定
        [self setDelegate:ref];
    }

#pragma mark - Private methods

    - (id)init {
        self = [super init];
        return self;
    }

@end
