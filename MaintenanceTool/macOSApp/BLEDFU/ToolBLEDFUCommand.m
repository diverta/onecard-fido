//
//  ToolBLEDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#import "ToolAppCommand.h"
#import "ToolBLECommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

// for DFU image file
#import "mcumgr_app_image.h"

// 更新対象アプリケーション＝version 0.4.0
#define DFU_UPD_TARGET_APP_VERSION      400

@interface ToolBLEDFUCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand  *toolAppCommand;

    // 更新イメージファイル名から取得したバージョン
    @property (nonatomic) NSString *updateVersionFromImage;
    // 認証器からHID経由で取得したバージョン、基板名
    @property (nonatomic) NSString *currentVersion;
    @property (nonatomic) NSString *currentBoardname;

@end

@implementation ToolBLEDFUCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            [self setToolAppCommand:(ToolAppCommand *)delegate];
        }
        // 内部保持バージョンをクリア
        [self setCurrentVersion:@""];
        [self setUpdateVersionFromImage:@""];
        return self;
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolBLECommandRef:(id)toolBLECommandRef {
        // 事前にBLE経由でバージョン情報を取得
        ToolBLECommand *toolBLECommand = (ToolBLECommand *)toolBLECommandRef;
        [toolBLECommand bleCommandWillProcess:COMMAND_BLE_GET_VERSION_INFO forCommand:self];
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

    - (void)setVersionInfoArrayFromResponse:(NSData *)response {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:response];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 取得したバージョン情報を内部保持
        [self setCurrentVersion:array[1]];
        [self setCurrentBoardname:array[2]];
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImageFile] == false) {
            [self notifyCancel];
            return;
        }
        // ツール同梱のイメージファイルのバージョンが、稼働中のファームウェアのバージョンより古い場合は処理を中止
        if ([self dfuImageIsAvailable] == false) {
            [self notifyCancel];
            return;
        }
        // TODO: 画面に制御を戻す
        [self notifyCancel];
    }

#pragma mark - Private methods

    - (bool)readDFUImageFile {
        // 更新イメージファイル（例：app_update.PCA10095.0.4.0.bin）の検索用文字列を生成
        NSString *binFileNamePrefix = [NSString stringWithFormat:@"app_update.%@.", [self currentBoardname]];
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
        // .binファイルからイメージを読込
        const char *zip_path = mcumgr_app_image_bin_filename();
        if (mcumgr_app_image_bin_read(zip_path) == false) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_READ_FAILED];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
             debugWithFormat:@"ToolBLEDFUCommand: DFU image file (%d bytes)", mcumgr_app_image_bin_size()];
        return true;
    }

    - (bool)dfuImageIsAvailable {
        // パッケージに同梱されている更新イメージファイル名からバージョンを取得
        NSString *update = [[NSString alloc] initWithUTF8String:mcumgr_app_image_bin_version()];
        // バージョンが取得できなかった場合は利用不可
        if ([update length] == 0) {
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE
                      informativeText:MSG_DFU_UPDATE_VERSION_UNKNOWN];
            return false;
        }
        // 認証器の現在バージョンが、更新イメージファイルのバージョンより新しい場合は利用不可
        int currentVersionDec = [ToolCommon calculateDecimalVersion:[self currentVersion]];
        int updateVersionDec = [ToolCommon calculateDecimalVersion:update];
        if (currentVersionDec > updateVersionDec) {
            NSString *informative = [NSString stringWithFormat:MSG_DFU_CURRENT_VERSION_ALREADY_NEW,
                                     [self currentVersion], update];
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE
                      informativeText:informative];
            return false;
        }
        // 認証器の現在バージョンが、所定バージョンより古い場合は利用不可（ソフトデバイスのバージョンが異なるため）
        if (currentVersionDec < DFU_UPD_TARGET_APP_VERSION) {
            NSString *informative = [NSString stringWithFormat:MSG_DFU_CURRENT_VERSION_OLD_USBBLD, update];
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE informativeText:informative];
            return false;
        }
        // 更新バージョンを保持
        [self setUpdateVersionFromImage:update];
        return true;
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
    }

    - (void)notifyCancel {
        // メイン画面に制御を戻す（ポップアップメッセージを表示しない）
        [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
    }

@end
