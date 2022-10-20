//
//  USBDFUACMCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/20.
//
#import "USBDFUACMCommand.h"

@interface USBDFUACMCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;

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
        // TODO: 仮の実装です。
        [[self delegate] didEstablishACMConnection:true];
    }

@end
