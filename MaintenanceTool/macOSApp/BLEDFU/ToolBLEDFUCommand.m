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
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

// for DFU image file
#import "mcumgr_app_image.h"

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
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        [self readDFUImageFile];
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

#pragma mark - Private methods

    - (bool)readDFUImageFile {
        // 更新イメージファイル（例：app_update.PCA10095.0.4.0.bin）の検索用文字列を生成
        NSString *binFileNamePrefix = [NSString stringWithFormat:@"app_update.%@.", [self strHWRev]];
        // 基板名に対応する更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImages:binFileNamePrefix] == false) {
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE informativeText:MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST];
            return false;
        }
        return true;
    }

    - (bool)readDFUImages:(NSString *)binFileNamePrefix {
        // リソースバンドル・ディレクトリーの絶対パスを取得
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        // .binファイル名を取得
        if (mcumgr_app_image_bin_filename_get([resourcePath UTF8String], [binFileNamePrefix UTF8String]) == false) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_FILENAME_CANNOT_GET];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
             debugWithFormat:@"ToolBLEDFUCommand: Firmware version %s, board name %s",
             mcumgr_app_image_bin_version(), mcumgr_app_image_bin_boardname()];
        return true;
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
    }

@end
