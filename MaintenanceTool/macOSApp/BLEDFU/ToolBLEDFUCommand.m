//
//  ToolBLEDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#import "ToolAppCommand.h"
#import "ToolBLEDFUCommand.h"

@interface ToolBLEDFUCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id delegate;

@end

@implementation ToolBLEDFUCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        return self;
    }

@end
