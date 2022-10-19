//
//  USBDFUTransferCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/19.
//
#import "DFUCommand.h"
#import "USBDFUTransferCommand.h"

@interface USBDFUTransferCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;

@end

@implementation USBDFUTransferCommand

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

    - (void)invokeTransferWithParamRef:(id)ref {
        // DFU処理のパラメーターを保持
        [self setCommandParameter:(DFUCommandParameter *)ref];
        // TODO: 仮の実装です。
        [NSThread sleepForTimeInterval:2.0];
        [[self delegate] transferCommandDidTerminate:true];
    }
 
@end
