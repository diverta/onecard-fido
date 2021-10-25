//
//  ToolBLEDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#import "BLEDFUProcessingWindow.h"
#import "BLEDFUStartWindow.h"
#import "ToolAppCommand.h"
#import "ToolBLECommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

// for DFU image file
#import "mcumgr_app_image.h"

// 処理タイムアウト（転送／反映チェック処理）
#define TIMEOUT_SEC_DFU_PROCESS         180.0

// 更新対象アプリケーション＝version 0.4.0
#define DFU_UPD_TARGET_APP_VERSION      400

@interface ToolBLEDFUCommand ()

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand     *toolAppCommand;
    // 画面の参照を保持
    @property (nonatomic) BLEDFUStartWindow        *bleDfuStartWindow;
    @property (nonatomic) BLEDFUProcessingWindow   *bleDfuProcessingWindow;
    // 非同期処理用のキュー（画面用／DFU処理用）
    @property (nonatomic) dispatch_queue_t          mainQueue;
    @property (nonatomic) dispatch_queue_t          subQueue;
    // 処理タイムアウト検知フラグ
    @property (nonatomic) bool                      needTimeoutMonitor;
    // バージョン更新判定フラグ
    @property (nonatomic) bool                      needCompareUpdateVersion;
    // 処理キャンセルフラグ
    @property (nonatomic) bool                      cancelFlag;

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
        // 画面のインスタンスを生成
        [self setBleDfuStartWindow:[[BLEDFUStartWindow alloc] initWithWindowNibName:@"BLEDFUStartWindow"]];
        [self setBleDfuProcessingWindow:[[BLEDFUProcessingWindow alloc] initWithWindowNibName:@"BLEDFUProcessingWindow"]];
        // メインスレッド／サブスレッドにバインドされるデフォルトキューを取得
        [self setMainQueue:dispatch_get_main_queue()];
        [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.bledfu", DISPATCH_QUEUE_SERIAL)];
        // 処理タイムアウト検知／バージョン更新判定フラグをリセット
        [self clearFlagsForProcess];
        return self;
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolBLECommandRef:(id)toolBLECommandRef {
        // 処理開始／進捗画面に親画面参照をセット
        [[self bleDfuStartWindow] setParentWindow:parentWindow];
        [[self bleDfuProcessingWindow] setParentWindow:parentWindow];
        // 事前にBLE経由でバージョン情報を取得
        ToolBLECommand *toolBLECommand = (ToolBLECommand *)toolBLECommandRef;
        [toolBLECommand bleCommandWillProcess:COMMAND_BLE_GET_VERSION_INFO forCommand:self];
    }

    - (void)toolBLECommandDidProcess:(Command)command response:(NSData *)response {
        switch (command) {
            case COMMAND_BLE_GET_VERSION_INFO:
                [self notifyFirmwareVersion:response];
                break;
            default:
                break;
        }
    }

    - (void)notifyFirmwareVersion:(NSData *)response {
        if (response == nil || [response length] == 0) {
            // エラーが発生したとみなし、画面に制御を戻す
            [ToolPopupWindow critical:MSG_DFU_SUB_PROCESS_FAILED informativeText:MSG_DFU_VERSION_INFO_GET_FAILED];
            [self cancelProcess];
            return;
        }
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:response];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 取得したバージョン情報を内部保持
        [self setCurrentVersion:array[1]];
        [self setCurrentBoardname:array[2]];
        // バージョン更新判定フラグがセットされている場合（ファームウェア反映待ち）
        if ([self needCompareUpdateVersion]) {
            // バージョン情報を比較して終了判定
            [self compareUpdateVersion];
        } else {
            // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
            [self resumeDfuProcessStart];
        }
    }

    - (void)compareUpdateVersion {
        // 処理タイムアウト検知／バージョン更新判定フラグをリセット
        [self clearFlagsForProcess];
        // TODO: バージョン情報を比較-->処理進捗画面に対し、処理結果を通知する
    }

    - (void)resumeDfuProcessStart {
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
        // 処理開始画面を表示
        [self dfuStartWindowWillOpen];
    }

#pragma mark - Interface for DFUStartWindow

    - (void)dfuStartWindowWillOpen {
        NSWindow *dialog = [[self bleDfuStartWindow] window];
        ToolBLEDFUCommand * __weak weakSelf = self;
        [[[self bleDfuStartWindow] parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuStartWindowDidClose:[self toolAppCommand] modalResponse:response];
        }];
        // バージョン情報を、ダイアログの該当欄に設定
        [[self bleDfuStartWindow] setWindowParameter:self
            currentVersion:[self currentVersion] updateVersion:[self updateVersionFromImage]];
    }

    - (void)dfuStartWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self bleDfuStartWindow] close];
        if (modalResponse == NSModalResponseCancel) {
            // キャンセルボタンがクリックされた場合は、ポップアップ画面を出さずに終了
            [self notifyCancel];
            return;
        }
        // DFU処理開始
        [self invokeDFUProcess];
    }

    - (void)invokeDFUProcess {
        // キャンセルフラグをクリア
        [self setCancelFlag:false];
        // 処理進捗画面（ダイアログ）をモーダルで表示
        [self bleDfuProcessingWindowWillOpen];
        // 処理進捗画面にDFU処理開始を通知
        [[self bleDfuProcessingWindow] commandDidStartDFUProcess];
        // サブスレッドでDFU処理を実行開始
        [self startDFUProcess];
    }

#pragma mark - Interface for DFUProcessingWindow

    - (void)bleDfuProcessingWindowWillOpen {
        NSWindow *dialog = [[self bleDfuProcessingWindow] window];
        ToolBLEDFUCommand * __weak weakSelf = self;
        [[[self bleDfuProcessingWindow] parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf bleDfuProcessingWindowDidClose:[self toolAppCommand] modalResponse:response];
        }];
    }

    - (void)bleDfuProcessingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        [[self bleDfuProcessingWindow] close];
        switch (modalResponse) {
            case NSModalResponseOK:
                [self notifyEndMessage:true];
                break;
            case NSModalResponseAbort:
                [self notifyEndMessage:false];
                break;
            default:
                // 処理をキャンセルする
                [self cancelProcess];
                break;
        }
    }

    - (void)cancelProcess {
        // キャンセルフラグを設定
        [self setCancelFlag:true];
        // 処理タイムアウト検知／バージョン更新判定フラグをリセット
        [self clearFlagsForProcess];
        // 処理進捗画面を閉じ、ポップアップ画面を出さずに終了
        [self notifyCancel];
    }

#pragma mark - Main process

    - (void)startDFUProcess {
        // 処理タイムアウト監視を開始
        [self startDFUTimeoutMonitor];
        // サブスレッドでDFU処理を実行
        dispatch_async([self subQueue], ^{
            // 処理失敗時は、処理進捗画面に対し通知
            if ([self performDFUProcess] == false) {
                [self notifyErrorToProcessingWindow];
            }
        });
        // メイン画面に開始メッセージを出力
        dispatch_async([self mainQueue], ^{
            [[self toolAppCommand] commandStartedProcess:COMMAND_BLE_DFU type:TRANSPORT_BLE];
        });
    }

    - (void)notifyErrorToProcessingWindow {
        dispatch_async([self mainQueue], ^{
            // 処理タイムアウト検知／バージョン更新判定フラグをリセット
            [self clearFlagsForProcess];
            // 処理進捗画面に対し、処理失敗の旨を通知する
            [[self bleDfuProcessingWindow] commandDidTerminateDFUProcess:false];
        });
    }

#pragma mark - Process timeout monitor

    - (void)startDFUTimeoutMonitor {
        // 処理タイムアウト監視を事前停止
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(DFUProcessDidTimeout) object:nil];
        // 処理タイムアウト監視を開始（指定秒後にタイムアウト）
        [self performSelector:@selector(DFUProcessDidTimeout) withObject:nil afterDelay:TIMEOUT_SEC_DFU_PROCESS];
        // 処理タイムアウト検知フラグを設定
        [self setNeedTimeoutMonitor:true];
    }

    - (void)DFUProcessDidTimeout {
        // 処理タイムアウト検知フラグが設定されている場合
        if ([self needTimeoutMonitor]) {
            // 処理タイムアウト検知／バージョン更新判定フラグをリセット
            [self clearFlagsForProcess];
            // 処理タイムアウトを検知したので、異常終了と判断
            [self notifyErrorMessage:MSG_DFU_PROCESS_TIMEOUT];
            // 処理進捗画面に対し、処理失敗の旨を通知する
            [[self bleDfuProcessingWindow] commandDidTerminateDFUProcess:false];
        }
    }

#pragma mark - DFU process

    - (bool)performDFUProcess {
        // DFUを実行
        [self notifyProgress:MSG_DFU_PROCESS_TRANSFER_IMAGE];
        if ([self performDFU] == false) {
            // DFU転送失敗時
            [self notifyErrorMessage:MSG_DFU_IMAGE_TRANSFER_FAILED];
            return false;

        } else if ([self cancelFlag]) {
            // 処理キャンセル時は、即時で画面に制御を戻す
            [self notifyMessage:MSG_DFU_IMAGE_TRANSFER_CANCELED];
            return true;

        } else {
            // DFU転送成功時は、バージョン更新判定フラグをセット
            [self notifyMessage:MSG_DFU_IMAGE_TRANSFER_SUCCESS];
            [self setNeedCompareUpdateVersion:true];
            [self notifyProgress:MSG_DFU_PROCESS_WAITING_UPDATE];
            return true;
        }
    }

    - (bool)performDFU {
        // TODO: 仮の実装です。
        for (int i = 0; i < 20; i++) {
            // 処理進捗画面でCancelボタンが押下された時は処理を中止
            if ([self cancelFlag]) {
                return true;
            }
            // １秒間ウェイト
            [NSThread sleepForTimeInterval:1.0];
        }
        return true;
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

#pragma mark - Private common methods

    - (void)clearFlagsForProcess {
        // 処理タイムアウト検知フラグをリセット
        [self setNeedTimeoutMonitor:false];
        // バージョン更新判定フラグをリセット
        [self setNeedCompareUpdateVersion:false];
    }

    - (void)notifyProgress:(NSString *)message {
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に進捗を通知
            [[self bleDfuProcessingWindow] commandDidNotifyDFUProcess:message];
        });
    }

    - (void)notifyMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] info:message];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
    }

    - (void)notifyEndMessage:(bool)success {
        // メイン画面に制御を戻す
        dispatch_async([self mainQueue], ^{
            [[self toolAppCommand] commandDidProcess:COMMAND_BLE_DFU result:success message:nil];
        });
    }

    - (void)notifyCancel {
        // メイン画面に制御を戻す（ポップアップメッセージを表示しない）
        dispatch_async([self mainQueue], ^{
            [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
        });
    }

@end
