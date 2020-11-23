//
//  ToolCCIDCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#import "AppDelegate.h"
#import "ToolCCIDCommand.h"
#import "ToolCCIDCommon.h"
#import "ToolCCIDHelper.h"
#import "ToolLogFile.h"

@interface ToolCCIDCommand ()

    // 画面の参照を保持
    @property (nonatomic, weak) AppDelegate *delegate;
    // CCIDインターフェース処理の参照を保持
    @property (nonatomic) ToolCCIDHelper    *toolCCIDHelper;
    // コマンドを保持
    @property (nonatomic) Command            command;

@end

@implementation ToolCCIDCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setDelegate:delegate];
        }
        // ToolCCIDHelperのインスタンスを生成
        [self setToolCCIDHelper:[[ToolCCIDHelper alloc] init]];
        return self;
    }

    - (void)ccidHelperWillProcess:(Command)command {
        // コマンドを待避
        [self setCommand:command];
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            default:
                [self doSelectApplication];
                break;
        }
    }

    - (void)ccidHelperDidProcess:(bool)success response:(NSData *)resp status:(uint16_t)sw {
        // コマンドに応じ、以下の処理に分岐
        switch ([self command]) {
            default:
                [self doResponseSelectApplication:resp status:sw];
                break;
        }
    }

#pragma mark - Command functions

    - (void)doSelectApplication {
        [[self toolCCIDHelper] setSendParameters:self ins:PIV_INS_SELECT_APPLICATION p1:0x04 p2:0x00 data:[self getPivAidData] le:0xff];
        [[self toolCCIDHelper] SCardSlotManagerWillBeginSession];
    }

    - (void)doResponseSelectApplication:(NSData *)response status:(uint16_t)sw {
        // for research
        [[ToolLogFile defaultLogger] debugWithFormat:@"doResponseSelectApplication: RESP[%@] SW[0x%04X]", response, sw];
    }

#pragma mark - Utility functions

    - (NSData *)getPivAidData {
        static uint8_t piv_aid[] = {0xa0, 0x00, 0x00, 0x03, 0x08};
        return [NSData dataWithBytes:piv_aid length:sizeof(piv_aid)];
    }

@end
