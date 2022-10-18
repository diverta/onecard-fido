//
//  USBDFUImage.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "nrf52_app_image.h"
#import "ToolCommon.h"
#import "ToolLogFile.h"
#import "USBDFUImage.h"

// 更新対象アプリケーション＝version 0.3.0
#define DFU_UPD_TARGET_APP_VERSION      300

@interface USBDFUImage ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;

@end

@implementation USBDFUImage

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

    - (bool)readDFUImageFile:(DFUCommandParameter *)commandParameter {
        // 更新イメージファイル（例：appkg.PCA10059_02.0.2.11.zip）の検索用文字列を生成
        NSString *zipFileNamePrefix = [NSString stringWithFormat:@"appkg.%@.", [commandParameter currentBoardname]];
        // 基板名に対応する更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImages:zipFileNamePrefix] == false) {
            [[self delegate] notifyCriticalErrorMessage:MSG_DFU_IMAGE_NOT_AVAILABLE informative:MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST];
            return false;
        }
        return true;
    }

    - (bool)readDFUImages:(NSString *)zipFileNamePrefix {
        // リソースバンドル・ディレクトリーの絶対パスを取得
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        // .zipファイル名を取得
        if (nrf52_app_image_zip_filename_get([resourcePath UTF8String], [zipFileNamePrefix UTF8String]) == false) {
            [[self delegate] notifyErrorMessage:MSG_DFU_IMAGE_FILENAME_CANNOT_GET];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: Firmware version %s, board name %s",
         nrf52_app_image_zip_version(), nrf52_app_image_zip_boardname()];
        // .zipファイルからイメージを読込
        const char *zip_path = nrf52_app_image_zip_filename();
        if (nrf52_app_image_zip_read(zip_path) == false) {
            [[self delegate] notifyErrorMessage:MSG_DFU_IMAGE_READ_FAILED];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: %@(%d bytes), %@(%d bytes)",
         @NRF52_APP_DAT_FILE_NAME, nrf52_app_image_dat_size(),
         @NRF52_APP_BIN_FILE_NAME, nrf52_app_image_bin_size()];
        return true;
    }

    - (bool)dfuImageIsAvailable:(DFUCommandParameter *)commandParameter {
        // パッケージに同梱されている更新イメージファイル名からバージョンを取得
        NSString *update = [[NSString alloc] initWithUTF8String:nrf52_app_image_zip_version()];
        // バージョンが取得できなかった場合は利用不可
        if ([update length] == 0) {
            [[self delegate] notifyCriticalErrorMessage:MSG_DFU_IMAGE_NOT_AVAILABLE informative:MSG_DFU_UPDATE_VERSION_UNKNOWN];
            return false;
        }
        // 認証器の現在バージョンが、更新イメージファイルのバージョンより新しい場合は利用不可
        int currentVersionDec = [ToolCommon calculateDecimalVersion:[commandParameter currentVersion]];
        int updateVersionDec = [ToolCommon calculateDecimalVersion:update];
        if (currentVersionDec > updateVersionDec) {
            NSString *informative = [NSString stringWithFormat:MSG_DFU_CURRENT_VERSION_ALREADY_NEW,
                                     [commandParameter currentVersion], update];
            [[self delegate] notifyCriticalErrorMessage:MSG_DFU_IMAGE_NOT_AVAILABLE informative:informative];
            return false;
        }
        // 認証器の現在バージョンが、所定バージョンより古い場合は利用不可（ソフトデバイスのバージョンが異なるため）
        if (currentVersionDec < DFU_UPD_TARGET_APP_VERSION) {
            NSString *informative = [NSString stringWithFormat:MSG_DFU_CURRENT_VERSION_OLD_USBBLD, update];
            [[self delegate] notifyCriticalErrorMessage:MSG_DFU_IMAGE_NOT_AVAILABLE informative:informative];
            return false;
        }
        // 更新バージョンを保持
        [commandParameter setUpdateVersionFromImage:update];
        return true;
    }

@end
