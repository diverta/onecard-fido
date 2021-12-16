//
//  ToolGPGCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#import "ToolAppCommand.h"
#import "ToolGPGCommand.h"

@interface ToolGPGCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand     *toolAppCommand;

@end

@implementation ToolGPGCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setToolAppCommand:(ToolAppCommand *)delegate];
        }
        return self;
    }

@end
