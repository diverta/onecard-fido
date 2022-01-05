//
//  ToolUSBDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/12/31.
//
#import <Foundation/Foundation.h>

#import "FIDODefines.h"
#import "debug_log.h"
#import "nrf52_app_image.h"
#import "ToolCDCHelper.h"
#import "ToolUSBDFUCommand.h"
#import "ToolHIDCommand.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"
#import "ToolAppCommand.h"
#import "DFUStartWindow.h"
#import "DFUProcessingWindow.h"

// 処理タイムアウト（転送／反映チェック処理）
#define TIMEOUT_SEC_DFU_PROCESS 30.0

// 応答タイムアウト
#define TIMEOUT_SEC_DFU_PING_RESPONSE  1.0
#define TIMEOUT_SEC_DFU_OPER_RESPONSE  3.0

// CDC ACM接続処理用の試行回数・インターバル
#define MAX_CNT_FOR_ACM_CONNECT        5
#define INTERVAL_SEC_FOR_ACM_CONNECT   1.0

// 詳細ログ出力
#define CDC_ACM_LOG_DEBUG false

// 新規導入対象基板名＝PCA10059_02（MDBT50Q Dongle rev2.1.2）
#define DFU_NEW_TARGET_BOARD_NAME       @"PCA10059_02"
// 新規導入対象ソフトデバイス＝version 7.2
#define DFU_NEW_TARGET_SOFTDEVICE_VER   7002000
// 更新対象アプリケーション＝version 0.3.0
#define DFU_UPD_TARGET_APP_VERSION      300

@interface ToolUSBDFUCommand ()

    @property (nonatomic)       ToolCDCHelper  *toolCDCHelper;
    @property (nonatomic, weak) ToolHIDCommand *toolHIDCommand;
    // 画面の参照を保持
    @property (nonatomic, weak) ToolAppCommand *toolAppCommand;
    @property (nonatomic) DFUStartWindow       *dfuStartWindow;
    @property (nonatomic) DFUProcessingWindow  *dfuProcessingWindow;
    // 非同期処理用のキュー（画面用／DFU処理用）
    @property (nonatomic) dispatch_queue_t mainQueue;
    @property (nonatomic) dispatch_queue_t subQueue;
    // ブートローダーモード遷移判定フラグ
    @property (nonatomic) bool needCheckBootloaderMode;
    // バージョン更新判定フラグ
    @property (nonatomic) bool needCompareUpdateVersion;
    // 更新イメージファイル名から取得したバージョン
    @property (nonatomic) NSString *updateVersionFromImage;
    // 認証器からHID経由で取得したバージョン、基板名
    @property (nonatomic) NSString *currentVersion;
    @property (nonatomic) NSString *currentBoardname;
    // タイムアウト監視フラグ
    @property (nonatomic) bool needTimeoutMonitor;

@end

@implementation ToolUSBDFUCommand

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
        // 画面のインスタンスを生成
        [self setDfuStartWindow:[[DFUStartWindow alloc]
                                 initWithWindowNibName:@"DFUStartWindow"]];
        [self setDfuProcessingWindow:[[DFUProcessingWindow alloc]
                                      initWithWindowNibName:@"DFUProcessingWindow"]];
        // ToolCDCHelperのインスタンスを生成
        [self setToolCDCHelper:[[ToolCDCHelper alloc] init]];
        // メインスレッド／サブスレッドにバインドされるデフォルトキューを取得
        [self setMainQueue:dispatch_get_main_queue()];
        [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.dfu", DISPATCH_QUEUE_SERIAL)];
        // ブートローダーモード遷移判定フラグをリセット
        [self setNeedCheckBootloaderMode:false];
        // バージョン更新判定フラグをリセット
        [self setNeedCompareUpdateVersion:false];
        return self;
    }

#pragma mark - Process timeout monitor

    - (void)startDFUTimeoutMonitor {
        // 処理タイムアウト監視を事前停止
        [NSObject cancelPreviousPerformRequestsWithTarget:self
                                                 selector:@selector(DFUProcessDidTimeout)
                                                   object:nil];
        // 処理タイムアウト監視を開始（指定秒後にタイムアウト）
        [self performSelector:@selector(DFUProcessDidTimeout)
                   withObject:nil afterDelay:TIMEOUT_SEC_DFU_PROCESS];
        // 処理タイムアウト検知フラグを設定
        [self setNeedTimeoutMonitor:true];
    }

    - (void)DFUProcessDidTimeout {
        // 処理タイムアウト検知フラグが設定されている場合
        if ([self needTimeoutMonitor]) {
            // バージョン更新判定フラグをリセット
            [self setNeedCompareUpdateVersion:false];
            // 処理タイムアウトを検知したので、異常終了と判断
            [self notifyErrorMessage:MSG_DFU_PROCESS_TIMEOUT];
            // 処理進捗画面に対し、処理失敗の旨を通知する
            [[self dfuProcessingWindow] commandDidTerminateDFUProcess:false];
        }
    }

#pragma mark - Call back from AppDelegate

    - (void)hidCommandDidDetectConnect:(Command)command forCommandRef:(id)ref {
        // 認証器の現在バージョンをクリア
        [self setCurrentVersion:@""];
        // バージョン更新判定フラグがセットされていない場合は終了
        if ([self needCompareUpdateVersion] == false) {
            return;
        }
        // USB HID経由でバージョン照会コマンドを実行
        if ([ref isMemberOfClass:[ToolUSBDFUCommand class]] == false) {
            return;
        }
        dispatch_async([self subQueue], ^{
            // サブスレッドでバージョン情報照会を実行 --> notifyFirmwareVersionが呼び出される
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_VERSION_FOR_DFU
                                               withData:nil forCommand:self];
        });
    }

    - (void)hidCommandDidDetectRemoval:(id)toolHIDCommandRef {
        // ブートローダーモード遷移判定フラグがセットされている場合（モード遷移完了待ち）
        if ([self needCheckBootloaderMode]) {
            // ブートローダーモード遷移判定フラグをリセット
            [self setNeedCheckBootloaderMode:false];
            // サブスレッドでDFU対象デバイスへの接続処理を実行
            [self EstablishDFUConnection];
        }
    }

#pragma mark - Call back from ToolHIDCommand

    - (void)hidCommandDidProcess:(Command)command CMD:(uint8_t)cmd response:(NSData *)response {
        switch (command) {
            case COMMAND_HID_GET_VERSION_FOR_DFU:
                [self notifyFirmwareVersion:response];
                break;
            case COMMAND_HID_BOOTLOADER_MODE:
                [self notifyBootloaderModeResponse:response CMD:cmd];
                break;
            default:
                break;
        }
    }

    - (void)notifyFirmwareVersion:(NSData *)versionInfoResponse {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:versionInfoResponse];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 認証器の現在バージョン、基板名を保持
        [self setCurrentVersion:array[1]];
        [self setCurrentBoardname:array[2]];
        // バージョン更新判定フラグがセットされている場合（ファームウェア反映待ち）
        if ([self needCompareUpdateVersion]) {
            // バージョン情報を比較して終了判定
            bool result = [self compareUpdateVersion:[self currentVersion]];
            // 処理進捗画面に対し、処理結果を通知する
            [[self dfuProcessingWindow] commandDidTerminateDFUProcess:result];
        } else {
            // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
            [self resumeDfuProcessStart];
        }
    }

    - (bool)compareUpdateVersion:(NSString *)update {
        // バージョン更新判定フラグをリセット
        [self setNeedCompareUpdateVersion:false];
        // 処理タイムアウト検知を不要とする
        [self setNeedTimeoutMonitor:false];
        // バージョン情報を比較
        char *fw_version = nrf52_app_image_zip_version();
        NSString *expected = [[NSString alloc] initWithUTF8String:fw_version];
        if (strcmp([update UTF8String], fw_version) == 0) {
            // バージョンが同じであればDFU処理は正常終了とする
            [self notifyMessage:
             [NSString stringWithFormat:MSG_DFU_FIRMWARE_VERSION_UPDATED, expected]];
            return true;
        } else {
            // バージョンが同じでなければ異常終了とする
            [self notifyErrorMessage:
             [NSString stringWithFormat:MSG_DFU_FIRMWARE_VERSION_UPDATED_FAILED,
              expected]];
            return false;
        }
    }

    - (void)notifyBootloaderModeResponse:(NSData *)message CMD:(uint8_t)cmd {
        // ブートローダーモード遷移コマンド成功時
        if (cmd == HID_CMD_BOOTLOADER_MODE) {
            // ブートローダーモード遷移判定フラグをセット --> hidCommandDidDetectRemovalが呼び出される
            [self setNeedCheckBootloaderMode:true];

        } else {
            // ブートローダーモード遷移コマンド失敗時は、ブートローダーモード遷移判定フラグをリセット
            [self setNeedCheckBootloaderMode:false];
            dispatch_async([self mainQueue], ^{
                // チェック結果を処理開始画面に引き渡す
                [[self dfuStartWindow]
                 commandDidChangeToBootloaderMode:false
                 errorMessage:MSG_DFU_TARGET_NOT_BOOTLOADER_MODE
                 informative:MSG_DFU_TARGET_NOT_SECURE_BOOTLOADER];
            });
        }
    }

#pragma mark - Interface for Main Process

    - (void)dfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolHIDCommandRef:(id)toolHIDCommandRef {
        // 処理前のチェック
        if ([self setupBeforeProcess:sender parentWindow:parentWindow] == false) {
            [self notifyCancel];
            return;
        }
        // 事前にバージョン情報取得を実行し、認証器の現在バージョンと基板名を取得 --> notifyFirmwareVersionが呼び出される
        [self setToolHIDCommand:(ToolHIDCommand *)toolHIDCommandRef];
        [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_GET_VERSION_FOR_DFU
                                           withData:nil forCommand:self];
    }

    - (bool)setupBeforeProcess:(id)sender parentWindow:(NSWindow *)parentWindow {
        // ダイアログの親ウィンドウを保持
        [[self dfuStartWindow] setParentWindow:parentWindow];
        [[self dfuProcessingWindow] setParentWindow:parentWindow];
        if ([[parentWindow sheets] count] > 0) {
            // すでにツール設定画面が開いている場合は終了
            return false;
        }
        return true;
    }

    - (void)resumeDfuProcessStart {
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImageFile] == false) {
            [self notifyCancel];
            return;
        }
        // 処理前のチェック
        if ([self versionCheckForDFU] == false) {
            // バージョンチェックが不正の場合はキャンセル
            [self notifyCancel];
            return;
        }
        if ([self dfuImageIsAvailable] == false) {
            // 更新イメージファイル名からバージョンが取得できていない場合は利用不可
            [self notifyCancel];
            return;
        }
        // 処理開始画面（ダイアログ）をモーダルで表示
        [self dfuStartWindowWillOpen];
    }

    - (bool)readDFUImageFile {
        // 更新イメージファイル（例：appkg.PCA10059_02.0.2.11.zip）の検索用文字列を生成
        NSString *zipFileNamePrefix = [NSString stringWithFormat:@"appkg.%@.", [self currentBoardname]];
        // 基板名に対応する更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImages:zipFileNamePrefix] == false) {
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE
                      informativeText:MSG_DFU_UPDATE_IMAGE_FILE_NOT_EXIST];
            return false;
        }
        return true;
    }

    - (bool)dfuImageIsAvailable {
        // パッケージに同梱されている更新イメージファイル名からバージョンを取得
        NSString *update = [[NSString alloc] initWithUTF8String:nrf52_app_image_zip_version()];
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

    - (bool)versionCheckForDFU {
        // HID経由で認証器の現在バージョンが取得できていない場合は利用不可
        if ([[self currentVersion] length] == 0) {
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE
                      informativeText:MSG_DFU_CURRENT_VERSION_UNKNOWN];
            return false;
        }
        return true;
    }

    - (void)dfuNewProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 新規導入対象の基板名を設定
        [self setCurrentBoardname:DFU_NEW_TARGET_BOARD_NAME];
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImageFile] == false) {
            [self notifyCancel];
            return;
        }
        // 処理前のチェック
        if ([self setupBeforeProcess:sender parentWindow:parentWindow] == false) {
            [self notifyCancel];
            return;
        }
        // DFU処理を開始するかどうかのプロンプトを表示
        if ([ToolPopupWindow promptYesNo:MSG_PROMPT_START_DFU_PROCESS
                         informativeText:MSG_COMMENT_START_DFU_PROCESS] == false) {
            [self notifyCancel];
            return;
        }
        // ファームウェア新規導入処理
        dispatch_async([self subQueue], ^{
            // サブスレッドで、DFU対象デバイスに対し、USB CDC ACM接続を実行
            bool result = [self searchACMDevicePath];
            dispatch_async([self mainQueue], ^{
                if (result) {
                    // DFU処理開始
                    [self invokeDFUProcess];
                } else {
                    // エラーメッセージを表示して終了
                    [ToolPopupWindow critical:MSG_DFU_IMAGE_NEW_NOT_AVAILABLE
                              informativeText:MSG_DFU_TARGET_CONNECTION_FAILED];
                    [self notifyCancel];
                }
            });
        });
    }

#pragma mark - Interface for DFUStartWindow

    - (void)dfuStartWindowWillOpen {
        NSWindow *dialog = [[self dfuStartWindow] window];
        ToolUSBDFUCommand * __weak weakSelf = self;
        [[[self dfuStartWindow] parentWindow] beginSheet:dialog
                                       completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuStartWindowDidClose:[self toolAppCommand] modalResponse:response];
        }];
        // バージョン情報を、ダイアログの該当欄に設定
        [[self dfuStartWindow] setWindowParameter:self
                                   currentVersion:[self currentVersion]
                                    updateVersion:[self updateVersionFromImage]];
    }

    - (void)dfuStartWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self dfuStartWindow] close];
        if (modalResponse == NSModalResponseCancel) {
            // キャンセルボタンがクリックされた場合は、ポップアップ画面を出さずに終了
            [self notifyCancel];
            return;
        }
        // DFU処理開始
        [self invokeDFUProcess];
    }

    - (void)invokeDFUProcess {
        // ソフトデバイスのバージョンが古い場合は処理を中止する
        if ([self checkSoftDeviceVersion] == false) {
            [self notifyCancel];
            return;
        }
        // 処理進捗画面（ダイアログ）をモーダルで表示
        [self dfuProcessingWindowWillOpen];
        // 処理進捗画面にDFU処理開始を通知
        [[self dfuProcessingWindow] commandDidStartDFUProcess];
        // サブスレッドでDFU処理を実行開始
        [self startDFUProcess];
    }

    - (bool)checkSoftDeviceVersion {
        // バージョン照会コマンドを実行
        uint32_t softDeviceVersion;
        if ([self sendFWVersionGetRequestOfFirmwareId:0x01 pVersionNumber:&softDeviceVersion] == false) {
            // DFU対象デバイスから切断
            [[self toolCDCHelper] disconnectDevice];
            return false;
        }
        [[ToolLogFile defaultLogger] debugWithFormat:@"ToolDFUCommand: SoftDevice version: %09d", softDeviceVersion];
        if (softDeviceVersion < DFU_NEW_TARGET_SOFTDEVICE_VER) {
            // ソフトデバイスのバージョンが所定バージョンより前であればエラーメッセージを表示
            [[self toolCDCHelper] disconnectDevice];
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NEW_NOT_AVAILABLE informativeText:MSG_DFU_TARGET_INVALID_SOFTDEVICE_VER];
            return false;
        }
        return true;
    }

    - (void)commandWillChangeToBootloaderMode {
        dispatch_async([self subQueue], ^{
            // ブートローダー遷移コマンドを実行 --> notifyBootloaderModeResponseが呼び出される
            [[self toolHIDCommand] hidHelperWillProcess:COMMAND_HID_BOOTLOADER_MODE
                                               withData:nil forCommand:self];
        });
    }

    - (bool)checkUSBHIDConnection {
        return [[self toolHIDCommand] checkUSBHIDConnection];
    }

    - (void)EstablishDFUConnection {
        dispatch_async([self subQueue], ^{
            // サブスレッドで、DFU対象デバイスに対し、USB CDC ACM接続を実行
            bool result = [self searchACMDevicePath];
            dispatch_async([self mainQueue], ^{
                // 処理結果を処理開始画面に引き渡す
                [[self dfuStartWindow]
                 commandDidChangeToBootloaderMode:result
                 errorMessage:MSG_DFU_TARGET_NOT_CONNECTED
                 informative:nil];
            });
        });
    }

    - (bool)searchACMDevicePath {
        // 最大５秒間繰り返す
        for (int i = 0; i < MAX_CNT_FOR_ACM_CONNECT; i++) {
            // １秒間ウェイト
            [NSThread sleepForTimeInterval:INTERVAL_SEC_FOR_ACM_CONNECT];
            // DFU対象デバイスに接続
            NSString *ACMDevicePath = [self getConnectedDevicePath];
            if (ACMDevicePath != nil) {
                // DFU対象デバイスに接続された場合はtrue
                [[ToolLogFile defaultLogger]
                 infoWithFormat:@"DFU target device found: %@", ACMDevicePath];
                return true;
            }
        }
        // 接続デバイスが見つからなかった場合はfalse
        [[ToolLogFile defaultLogger] error:@"DFU target device not found"];
        return false;
    }

#pragma mark - Interface for DFUProcessingWindow

    - (void)dfuProcessingWindowWillOpen {
        NSWindow *dialog = [[self dfuProcessingWindow] window];
        ToolUSBDFUCommand * __weak weakSelf = self;
        [[[self dfuProcessingWindow] parentWindow] beginSheet:dialog
                                            completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuProcessingWindowDidClose:[self toolAppCommand] modalResponse:response];
        }];
    }

    - (void)dfuProcessingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        [[self dfuProcessingWindow] close];
        switch (modalResponse) {
            case NSModalResponseOK:
                [self notifyEndMessage:true];
                break;
            case NSModalResponseAbort:
                [self notifyEndMessage:false];
                break;
            default:
                // 処理進捗画面を閉じ、ポップアップ画面を出さずに終了
                [self notifyCancel];
                break;
        }
    }

    - (void)notifyProgress:(NSString *)message {
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に進捗を通知
            [[self dfuProcessingWindow] commandDidNotifyDFUProcess:message];
        });
    }

#pragma mark - Main process

    - (void)startDFUProcess {
        // 処理タイムアウト監視を開始
        [self startDFUTimeoutMonitor];
        dispatch_async([self subQueue], ^{
            // サブスレッドでDFU処理を実行
            if ([self performDFUProcess] == false) {
                // 処理失敗時は、処理進捗画面に対し通知
                [self notifyErrorToProcessingWindow];
            }
        });
        dispatch_async([self mainQueue], ^{
            // メイン画面に開始メッセージを出力
            [[self toolAppCommand] commandStartedProcess:COMMAND_USB_DFU type:TRANSPORT_HID];
        });
    }

    - (void)notifyErrorToProcessingWindow {
        dispatch_async([self mainQueue], ^{
            // 処理失敗時は処理タイムアウト検知を不要とする
            [self setNeedTimeoutMonitor:false];
            // 処理進捗画面に対し、処理失敗の旨を通知する
            [[self dfuProcessingWindow] commandDidTerminateDFUProcess:false];
        });
    }

    - (bool)performDFUProcess {
        // DFUを実行
        [self notifyProgress:MSG_DFU_PROCESS_TRANSFER_IMAGE];
        bool ret = [self performDFU];
        // DFU対象デバイスから切断
        [[self toolCDCHelper] disconnectDevice];
        if (ret == false) {
            // DFU転送失敗時
            [self notifyErrorMessage:MSG_DFU_IMAGE_TRANSFER_FAILED];
        } else {
            // DFU転送成功時は、バージョン更新判定フラグをセット
            [self notifyMessage:MSG_DFU_IMAGE_TRANSFER_SUCCESS];
            [self setNeedCompareUpdateVersion:true];
            [self notifyProgress:MSG_DFU_PROCESS_WAITING_UPDATE];
            // 再接続まで待機 --> hidCommandDidDetectConnect が呼び出される
            [[self toolHIDCommand] hidHelperWillDetectConnect:COMMAND_USB_DFU forCommand:self];
        }
        return ret;
    }

#pragma mark - Sub process

    - (void)notifyMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] info:message];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
    }

    - (void)notifyEndMessage:(bool)success {
        // メイン画面に制御を戻す
        dispatch_async([self mainQueue], ^{
            [[self toolAppCommand] commandDidProcess:COMMAND_USB_DFU result:success message:nil];
        });
    }

    - (void)notifyCancel {
        // メイン画面に制御を戻す（ポップアップメッセージを表示しない）
        dispatch_async([self mainQueue], ^{
            [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
        });
    }

    - (bool)readDFUImages:(NSString *)zipFileNamePrefix {
        // リソースバンドル・ディレクトリーの絶対パスを取得
        NSString *resourcePath = [[NSBundle mainBundle] resourcePath];
        // .zipファイル名を取得
        if (nrf52_app_image_zip_filename_get([resourcePath UTF8String], [zipFileNamePrefix UTF8String]) == false) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_FILENAME_CANNOT_GET];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: Firmware version %s, board name %s",
         nrf52_app_image_zip_version(), nrf52_app_image_zip_boardname()];
        // .zipファイルからイメージを読込
        const char *zip_path = nrf52_app_image_zip_filename();
        if (nrf52_app_image_zip_read(zip_path) == false) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_READ_FAILED];
            return false;
        }
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: %@(%d bytes), %@(%d bytes)",
         @NRF52_APP_DAT_FILE_NAME, nrf52_app_image_dat_size(),
         @NRF52_APP_BIN_FILE_NAME, nrf52_app_image_bin_size()];
        return true;
    }

    - (bool)performDFU {
        // DFU対象デバイスの通知設定
        if ([self sendSetReceiptRequest] == false) {
            return false;
        }
        // DFU対象デバイスからMTUを取得
        if ([self sendGetMtuRequest] == false) {
            return false;
        }
        // datイメージを転送
        if ([self transferDFUImage:NRF_DFU_BYTE_OBJ_INIT_CMD
                         imageData:nrf52_app_image_dat()
                         imageSize:nrf52_app_image_dat_size()] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debug:@"ToolDFUCommand: update init command object done"];
        // binイメージを転送
        if ([self transferDFUImage:NRF_DFU_BYTE_OBJ_DATA
                         imageData:nrf52_app_image_bin()
                         imageSize:nrf52_app_image_bin_size()] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger] debug:@"ToolDFUCommand: update data object done"];
        return true;
    }

    - (bool)transferDFUImage:(uint8_t)objectType
                   imageData:(uint8_t *)data imageSize:(size_t)size {
        // １回あたりの送信データ最大長を取得
        size_t maxCreateSize;
        if ([self sendSelectObjectRequest:objectType pMaxCreateSize:&maxCreateSize] == false) {
            return false;
        }
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"ToolDFUCommand: object select size=%d, create size max=%d",
         size, maxCreateSize];
        // データを分割送信
        size_t remaining = size;
        size_t alreadySent = 0;
        while (remaining > 0) {
            // 送信サイズを通知
            size_t sendSize = (maxCreateSize < remaining) ? maxCreateSize : remaining;
            if ([self sendCreateObjectRequest:objectType imageSize:sendSize] == false) {
                return false;
            }
            // データを送信
            uint8_t *sendData = data + alreadySent;
            if ([self sendWriteCommandObjectRequest:objectType
                                          imageData:sendData imageSize:sendSize] == false) {
                return false;
            }
            // 送信データのチェックサムを検証
            alreadySent += sendSize;
            if ([self sendGetCrcRequest:objectType imageSize:alreadySent] == false) {
                return false;
            }
            // 送信データをコミット
            if ([self sendExecuteObjectRequest] == false) {
                return false;
            }
            // 未送信サイズを更新
            remaining -= sendSize;
        }
        return true;
    }

    - (bool)sendPingRequest:(uint8_t)id {
        // PING 09 01 C0 -> 60 09 01 01 C0
        uint8_t pingRequest[] = {NRF_DFU_OP_PING, id, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:pingRequest length:sizeof(pingRequest)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_PING_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // IDを比較
        uint8_t *pingResponse = (uint8_t *)[response bytes];
        return (pingResponse[3] == id);
    }

    - (bool)sendFWVersionGetRequestOfFirmwareId:(uint8_t)fwid pVersionNumber:(uint32_t *)pNumber {
        // GET FW VERSION 0b 01 C0 -> 60 0B 01 00 90 D7 6A 00 ... C0
        static uint8_t request[] = {NRF_DFU_OP_FIRMWARE_VERSION, 0x00, NRF_DFU_BYTE_EOM};
        request[1] = fwid;
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスから、バージョン番号を取得（5〜8バイト目）
        *pNumber = [self convertLEBytesToUint32:[response bytes] offset:4];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
    }

    - (bool)sendSetReceiptRequest {
        // SET RECEIPT 02 00 00 C0 -> 60 02 01 C0
        static uint8_t request[] = {
            NRF_DFU_OP_RECEIPT_NOTIF_SET, 0x00, 0x00, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
    }

    - (bool)sendGetMtuRequest {
        // Get the preferred MTU size on the request.
        // GET MTU 07 C0 -> 60 07 01 83 00 C0
        static uint8_t mtuRequest[] = {NRF_DFU_OP_MTU_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:mtuRequest length:sizeof(mtuRequest)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスからMTUを取得（4〜5バイト目）
        uint16_t mtu = [self convertLEBytesToUint16:[response bytes] offset:3];
        size_t mtu_size = usb_dfu_object_set_mtu(mtu);
        [[ToolLogFile defaultLogger] debugWithFormat:@"ToolDFUCommand: MTU=%d", mtu_size];
        return true;
    }

    - (bool)sendSelectObjectRequest:(uint8_t)objectType pMaxCreateSize:(size_t *)pMaxCreateSize {
        // SELECT OBJECT 06 xx C0 -> 60 06 xx 00 01 00 00 00 00 00 00 00 00 00 00 C0
        uint8_t request[] = {
            NRF_DFU_OP_OBJECT_SELECT, objectType,
            NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスから、イメージの最大送信可能サイズを取得（4〜7バイト目）
        *pMaxCreateSize = (size_t)[self convertLEBytesToUint32:[response bytes] offset:3];
        // チェックサムを初期化
        usb_dfu_object_checksum_reset();
        return true;
    }

    - (bool)sendCreateObjectRequest:(uint8_t)objectType imageSize:(size_t)imageSize {
        // CREATE OBJECT 01 xx 87 00 00 00 C0 -> 60 01 01 C0
        uint8_t createObjectRequest[] = {
            NRF_DFU_OP_OBJECT_CREATE, objectType, 0x00, 0x00, 0x00, 0x00,
            NRF_DFU_BYTE_EOM};
        uint32_t commandObjectLen = (uint32_t)imageSize;
        [self convertUint32ToLEBytes:commandObjectLen data:createObjectRequest offset:2];
        
        NSData *data = [NSData dataWithBytes:createObjectRequest length:sizeof(createObjectRequest)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
    }

    - (bool)sendWriteCommandObjectRequest:(uint8_t)objectType
                                imageData:(uint8_t *)data imageSize:(size_t)size {
        // オブジェクト種別に対応するデータ／サイズを設定
        usb_dfu_object_frame_init(data, size);
        // 送信フレームを生成
        while (usb_dfu_object_frame_prepare()) {
            // フレームを送信
            NSData *frame = [NSData dataWithBytes:usb_dfu_object_frame_data()
                                          length:usb_dfu_object_frame_size()];
            if ([self sendRequestData:frame] == false) {
                return false;
            }
#if CDC_ACM_LOG_DEBUG
            // ログ出力
            [[ToolLogFile defaultLogger]
             debugWithFormat:@"CDC ACM Send (%d bytes)", [frame length]];
#endif
        }
        return true;
    }

    - (bool)sendGetCrcRequest:(uint8_t)objectType imageSize:(size_t)imageSize {
        // CRC GET 03 C0 -> 60 03 01 87 00 00 00 38 f4 97 72 C0
        uint8_t request[] = {NRF_DFU_OP_CRC_GET, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        if ([self assertDFUResponseSuccess:response] == false) {
            return false;
        }
        // レスポンスデータから、エスケープシーケンスを取り除く
        NSData *respUnesc = [self unescapeResponseData:response];

        // 送信データ長を検証
        size_t recvSize = (size_t)[self convertLEBytesToUint32:[respUnesc bytes] offset:3];
        if (recvSize != imageSize) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"ToolDFUCommand: send object %d failed (expected %d bytes, recv %d bytes)",
             objectType, imageSize, recvSize];
            return false;
        }
        // チェックサムを検証
        uint32_t checksum = [self convertLEBytesToUint32:[respUnesc bytes] offset:7];
        if (checksum != usb_dfu_object_checksum_get()) {
            [[ToolLogFile defaultLogger]
             errorWithFormat:@"ToolDFUCommand: send object %d failed (checksum error)",
             objectType];
            return false;
        }
        return true;
    }

    - (bool)sendExecuteObjectRequest {
        // EXECUTE OBJECT 04 C0 -> 60 04 01 C0
        static uint8_t request[] = {NRF_DFU_OP_OBJECT_EXECUTE, NRF_DFU_BYTE_EOM};
        NSData *data = [NSData dataWithBytes:request length:sizeof(request)];
        NSData *response = [self sendRequest:data timeoutSec:TIMEOUT_SEC_DFU_OPER_RESPONSE];
        // レスポンスを検証
        return [self assertDFUResponseSuccess:response];
    }

    - (NSData *)unescapeResponseData:(NSData *)response {
        uint8_t c;
        NSMutableData *unescaped = [[NSMutableData alloc] init];
        
        uint8_t *data = (uint8_t *)[response bytes];
        size_t   size = [response length];
        
        bool escapeChar = false;
        for (size_t i = 0; i < size; i++) {
            c = data[i];
            if (c == 0xdb) {
                escapeChar = true;
            } else {
                if (escapeChar) {
                    escapeChar = false;
                    if (c == 0xdc) {
                        c = 0xc0;
                    } else if (c == 0xdd) {
                        c = 0xdb;
                    }
                }
                [unescaped appendBytes:&c length:sizeof(c)];
            }
        }
        return unescaped;
    }

    - (uint16_t)convertLEBytesToUint16:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint16_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8);
        return uint;
    }

    - (uint32_t)convertLEBytesToUint32:(const void *)data offset:(uint16_t)offset {
        uint8_t *bytes = (uint8_t *)data;
        uint32_t uint = bytes[offset] | ((uint16_t)bytes[offset + 1] << 8)
            | ((uint32_t)bytes[offset + 2] << 16) | ((uint32_t)bytes[offset + 3] << 24);
        return uint;
    }

    - (void)convertUint32ToLEBytes:(uint32_t)uint data:(uint8_t *)data offset:(uint16_t)offset {
        uint8_t *bytes = data + offset;
        for (uint8_t i = 0; i < 4; i++) {
            *bytes++ = uint & 0xff;
            uint = uint >> 8;
        }
    }

    - (NSData *)sendRequest:(NSData *)data timeoutSec:(double)timeout {
#if CDC_ACM_LOG_DEBUG
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Send (%d bytes):", [data length]];
        [[ToolLogFile defaultLogger] hexdump:data];
#endif
        // データ送信
        if ([self sendRequestData:data] == false) {
            return nil;
        }
        // データを受信
        NSData *dataRecv = [[self toolCDCHelper] readFromDevice:timeout];
        if (dataRecv == nil) {
            return nil;
        }
#if CDC_ACM_LOG_DEBUG
        // ログ出力
        [[ToolLogFile defaultLogger]
         debugWithFormat:@"CDC ACM Recv (%d bytes):", [dataRecv length]];
        [[ToolLogFile defaultLogger] hexdump:dataRecv];
#endif
        return dataRecv;
    }

    - (bool)sendRequestData:(NSData *)data {
        // データを送信
        return [[self toolCDCHelper] writeToDevice:data];
    }

    - (bool)assertDFUResponseSuccess:(NSData *)response {
        // レスポンスを検証
        if (response == nil) {
            return false;
        }
        // ステータスコードを参照し、処理が成功したかどうかを判定
        uint8_t *responseBytes = (uint8_t *)[response bytes];
        return (responseBytes[2] == NRF_DFU_BYTE_RESP_SUCCESS);
    }

    - (NSString *)getConnectedDevicePath {
        // 接続されているUSB CDC ACMデバイスのパスを走査
        NSArray *ACMDevicePathList = [[self toolCDCHelper] createACMDevicePathList];
        if (ACMDevicePathList != nil) {
            uint8_t id = 0xac;
            for (NSString *ACMDevicePath in ACMDevicePathList) {
                // 接続を実行
                if ([[self toolCDCHelper] connectDeviceTo:ACMDevicePath] == false) {
                    return nil;
                }
                // DFU PINGを実行
                if ([self sendPingRequest:id]) {
                    // 成功した場合は、接続された状態で、デバイスのパスを戻す
                    return ACMDevicePath;
                } else {
                    // 失敗した場合は、接続を閉じ、次のデバイスに移る
                    [[self toolCDCHelper] disconnectDevice];
                }
            }
        }
        // デバイスが未接続の場合は、NULLを戻す
        return nil;
    }

@end
