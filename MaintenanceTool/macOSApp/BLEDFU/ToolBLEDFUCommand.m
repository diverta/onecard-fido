//
//  ToolBLEDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#import "ToolAppCommand.h"
#import "ToolBLECommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"

@interface ToolBLEDFUCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id delegate;

    // バージョン情報
    @property (nonatomic) NSString *strFWRev;
    @property (nonatomic) NSString *strHWRev;

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

    - (void)getVersionInfoWithCommand:(ToolBLECommand *)toolBLECommand {
        // 事前にBLE経由でバージョン情報を取得
        [toolBLECommand bleCommandWillProcess:COMMAND_BLE_GET_VERSION_INFO forCommand:self];
    }

    - (void)setVersionInfoArrayFromResponse:(NSData *)response {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:response];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 取得したバージョン情報を内部保持
        [self setStrFWRev:array[1]];
        [self setStrHWRev:array[2]];
        [[ToolLogFile defaultLogger] debugWithFormat:@"%@ %@", [self strFWRev], [self strHWRev]];
        // TODO: 画面に制御を戻す
        [self commandDidTerminate:COMMAND_NONE result:true message:nil];
    }

    - (void)toolBLECommandDidProcess:(Command)command response:(NSData *)response {
        switch (command) {
            case COMMAND_BLE_GET_VERSION_INFO:
                [self setVersionInfoArrayFromResponse:response];
                break;
            default:
                break;
        }
    }

    - (void)commandDidTerminate:(Command)command result:(bool)result message:(NSString *)message {
        // ホーム画面に制御を戻す
        ToolAppCommand *toolAppCommand = (ToolAppCommand *)[self delegate];
        [toolAppCommand commandDidProcess:command result:result message:message];
    }

@end
