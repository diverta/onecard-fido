//
//  USBDFUImage.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "DFUCommand.h"
#import "nrf52_app_image.h"
#import "ToolLogFile.h"
#import "USBDFUImage.h"

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

@end
