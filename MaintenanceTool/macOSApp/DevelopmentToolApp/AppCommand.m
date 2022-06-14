//
//  AppCommand.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/14.
//
#import "AppCommand.h"

@interface AppCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;

@end

@implementation AppCommand

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
