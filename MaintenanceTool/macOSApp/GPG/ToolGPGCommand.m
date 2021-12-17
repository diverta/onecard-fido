//
//  ToolGPGCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#import "ToolAppCommand.h"
#import "ToolGPGCommand.h"
#import "ToolLogFile.h"

@interface ToolGPGCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand         *toolAppCommand;
    // コマンド種別を保持
    @property (nonatomic) Command                       command;
    // コマンドからの応答データを保持
    @property (nonatomic) NSMutableArray<NSData *>     *commandOutput;
    // 生成された作業用フォルダー名称を保持
    @property (nonatomic) NSString                     *tempFolderPath;

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

    - (void)doRequestMakeTempFolder {
        // 作業用フォルダーをPC上に生成
        NSString *path = @"/usr/bin/mktemp";
        NSArray *args = @[@"-d"];
        [self doRequestCommandLine:COMMAND_GPG_MAKE_TEMP_FOLDER commandPath:path commandArgs:args];
    }

    - (void)doResponseMakeTempFolder:(NSMutableArray<NSString *> *)response {
        // 生成された作業用フォルダー名称を保持
        if ([response count] == 1) {
            [self setTempFolderPath:[response objectAtIndex:0]];
            [[ToolLogFile defaultLogger] debugWithFormat:@"Temp folder path: %@", [self tempFolderPath]];
        }
    }

#pragma mark - Command line processor

    - (void)doRequestCommandLine:(Command)command commandPath:(NSString*)path commandArgs:(NSArray*)args {
        // コマンド種別を保持
        [self setCommand:command];
        // 標準入力用
        NSTask *task = [[NSTask alloc] init];
        [task setStandardInput:[NSPipe pipe]];
        // 標準出力用
        [task setStandardOutput:[NSPipe pipe]];
        [task setStandardError:[task standardOutput]];
        // 実行するコマンドのパスと引数を設定
        [task setLaunchPath:path];
        [task setArguments:args];
        // 応答文字列の格納用配列を初期化
        [self setCommandOutput:[[NSMutableArray alloc] init]];
        // コマンドからの応答を待機
        ToolGPGCommand * __weak weakSelf = self;
        [[[task standardOutput] fileHandleForReading] waitForDataInBackgroundAndNotify];
        [[NSNotificationCenter defaultCenter]
            addObserverForName:NSFileHandleDataAvailableNotification
                        object:[[task standardOutput] fileHandleForReading] queue:nil
                    usingBlock:^(NSNotification *notification) {
            [weakSelf extractOutputStringWith:task];
        }];
        // コマンドを実行
        [task launch];
        [task waitUntilExit];
        [[ToolLogFile defaultLogger] debugWithFormat:@"Command executed: %@", path];
    }

    - (void)extractOutputStringWith:(NSTask *)task {
        // 応答がこれ以上無ければ終了
        NSData *output = [[[task standardOutput] fileHandleForReading] availableData];
        if (output == nil || [output length] == 0) {
            [self commandDidTerminated];
            return;
        }
        // 応答データを配列に保持
        [[self commandOutput] addObject:output];
        // 次の応答があれば待機
        [[[task standardOutput] fileHandleForReading] waitForDataInBackgroundAndNotify];
    }

    - (void)commandDidTerminated {
        // コマンド実行完了
        [[ToolLogFile defaultLogger] debug:@"Command terminated"];
        // 応答データを配列に抽出
        NSMutableArray<NSString *> *outputArray = [[NSMutableArray alloc] init];
        for (NSData *data in [self commandOutput]) {
            // データ末尾に改行文字があれば削除
            NSData *output = data;
            uint8_t *bytes = (uint8_t *)[data bytes];
            if (bytes[[data length] - 1] == 0x0a) {
                output = [data subdataWithRange:NSMakeRange(0, [data length] - 1)];
            }
            // データを文字列に変換し、配列に格納
            NSString *outStr = [[NSString alloc] initWithData:output encoding:NSUTF8StringEncoding];
            [outputArray addObject:outStr];
        }
        // レスポンスを処理
        switch ([self command]) {
            case COMMAND_GPG_MAKE_TEMP_FOLDER:
                [self doResponseMakeTempFolder:outputArray];
                break;
            default:
                return;
        }
    }

@end
