//
//  OATHCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//
#import "OATHCommand.h"

@implementation OATHCommandParameter

@end

@interface OATHCommand ()

    // 処理のパラメーターを保持
    @property (nonatomic) OATHCommandParameter     *commandParameter;

@end

@implementation OATHCommand

    - (id)initWithDelegate:(id)delegate {
        self = [super initWithDelegate:delegate];
        if (self) {
            // ヘルパークラスのインスタンスを生成
            [self setCommandParameter:[[OATHCommandParameter alloc] init]];
        }
        return self;
    }

@end
