//
//  ToolBLEDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/19.
//
#import "BLEDFUDefine.h"
#import "BLEDFUProcessingWindow.h"
#import "BLEDFUStartWindow.h"
#import "ToolAppCommand.h"
#import "ToolBLECommand.h"
#import "ToolBLEDFUCommand.h"
#import "ToolBLESMPCommand.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"

// for DFU image file
#import "mcumgr_app_image.h"

// for CBOR decode
#include "debug_log.h"
#include "mcumgr_cbor_decode.h"

// 処理ステータス
typedef enum : NSInteger {
    BLEDFU_ST_NONE = 0,
    BLEDFU_ST_GET_CURRENT_VERSION,
    BLEDFU_ST_UPLOAD_PROCESS,
    BLEDFU_ST_CANCELED,
    BLEDFU_ST_RESET_DONE,
    BLEDFU_ST_WAIT_FOR_BOOT,
    BLEDFU_ST_CHECK_UPDATE_VERSION,
} BLEDFUStatus;

@interface ToolBLEDFUCommand () <ToolBLESMPCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) ToolAppCommand     *toolAppCommand;
    @property (nonatomic, weak) ToolBLECommand     *toolBLECommand;
    // トランザクションクラスの参照を保持
    @property (nonatomic) ToolBLESMPCommand        *toolBLESMPCommand;
    // 画面の参照を保持
    @property (nonatomic) BLEDFUStartWindow        *bleDfuStartWindow;
    @property (nonatomic) BLEDFUProcessingWindow   *bleDfuProcessingWindow;
    // 非同期処理用のキュー（画面用／DFU処理用）
    @property (nonatomic) dispatch_queue_t          mainQueue;
    @property (nonatomic) dispatch_queue_t          subQueue;
    // 処理タイムアウト検知フラグ
    @property (nonatomic) bool                      needTimeoutMonitor;
    // 処理ステータス
    @property (nonatomic) BLEDFUStatus              bleDfuStatus;
    // デバイス接続の切断理由を保持
    @property (nonatomic) bool                      disconnectByError;
    // 転送するイメージデータを保持
    @property (nonatomic) NSData                   *imageToUpload;

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
        // トランザクションクラスを生成
        [self setToolBLESMPCommand:[[ToolBLESMPCommand alloc] initWithDelegate:self]];
        // 画面のインスタンスを生成
        [self setBleDfuStartWindow:[[BLEDFUStartWindow alloc] initWithWindowNibName:@"BLEDFUStartWindow"]];
        [self setBleDfuProcessingWindow:[[BLEDFUProcessingWindow alloc] initWithWindowNibName:@"BLEDFUProcessingWindow"]];
        // メインスレッド／サブスレッドにバインドされるデフォルトキューを取得
        [self setMainQueue:dispatch_get_main_queue()];
        [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.bledfu", DISPATCH_QUEUE_SERIAL)];
        // ステータスを初期化
        [self setBleDfuStatus:BLEDFU_ST_NONE];
        return self;
    }

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow toolBLECommandRef:(id)toolBLECommandRef {
        // 処理開始／進捗画面に親画面参照をセット
        [[self bleDfuStartWindow] setParentWindow:parentWindow];
        [[self bleDfuProcessingWindow] setParentWindow:parentWindow];
        // ステータスを更新（現在バージョン照会）
        [self setBleDfuStatus:BLEDFU_ST_GET_CURRENT_VERSION];
        // 事前にBLE経由でバージョン情報を取得
        [self setToolBLECommand:(ToolBLECommand *)toolBLECommandRef];
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_BLE_GET_VERSION_INFO forCommand:self];
    }

    - (void)toolBLECommandDidProcess:(Command)command success:(bool)success response:(NSData *)response {
        switch (command) {
            case COMMAND_BLE_GET_VERSION_INFO:
                // バージョン更新判定の場合（ファームウェア反映待ち）
                if ([self bleDfuStatus] == BLEDFU_ST_CHECK_UPDATE_VERSION) {
                    [self notifyFirmwareVersionForComplete:success response:response];
                }
                // 現在バージョン照会の場合（処理開始画面の表示前）
                if ([self bleDfuStatus] == BLEDFU_ST_GET_CURRENT_VERSION) {
                    [self notifyFirmwareVersionForStart:success response:response];
                }
                break;
            default:
                break;
        }
    }

    - (void)notifyFirmwareVersionForComplete:(bool)success response:(NSData *)response {
        if (success == false || response == nil || [response length] < 2) {
            // エラーが発生したとみなす
            [self notifyErrorMessage:MSG_DFU_VERSION_INFO_GET_FAILED];
            // BLE接続を切断
            [self doDisconnectByError:true];
            return;
        }
        // 戻りメッセージからバージョン情報を抽出し内部保持
        [self extractVersionAndBoardnameFrom:response];
        // バージョン情報を比較して終了判定
        [self compareUpdateVersion];
    }

    - (void)notifyFirmwareVersionForStart:(bool)success response:(NSData *)response {
        if (success == false || response == nil || [response length] < 2) {
            // エラーが発生した場合は、メッセージをログ出力／ポップアップ表示したのち、画面に制御を戻す
            [[ToolLogFile defaultLogger] error:MSG_DFU_VERSION_INFO_GET_FAILED];
            [ToolPopupWindow critical:MSG_DFU_VERSION_INFO_GET_FAILED informativeText:nil];
            [self notifyProcessCanceled];
            return;
        }
        // 戻りメッセージからバージョン情報を抽出し内部保持
        [self extractVersionAndBoardnameFrom:response];
        // 認証器の現在バージョンと基板名が取得できたら、ファームウェア更新画面を表示
        [self resumeDfuProcessStart];
    }

    - (void)extractVersionAndBoardnameFrom:(NSData *)response {
        // 戻りメッセージから、取得情報CSVを抽出
        NSData *responseBytes = [ToolCommon extractCBORBytesFrom:response];
        NSString *responseCSV = [[NSString alloc] initWithData:responseBytes encoding:NSASCIIStringEncoding];
        // 情報取得CSVからバージョン情報を抽出
        NSArray<NSString *> *array = [ToolCommon extractValuesFromVersionInfo:responseCSV];
        // 取得したバージョン情報を内部保持
        [self setCurrentVersion:array[1]];
        [self setCurrentBoardname:array[2]];
    }

    - (void)compareUpdateVersion {
        // 処理タイムアウト検知／バージョン更新判定フラグをリセット
        [self clearFlagsForProcess];
        // バージョン情報を比較
        bool ret = [self compareUpdateCurrentVersionToAppImage:[self currentVersion]];
        // 処理進捗画面に対し、処理結果を通知する
        [[self bleDfuProcessingWindow] commandDidTerminateDFUProcess:ret];
    }

    - (bool)compareUpdateCurrentVersionToAppImage:(NSString *)update {
        // バージョン情報を比較
        char *fw_version = mcumgr_app_image_bin_version();
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

    - (void)resumeDfuProcessStart {
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        if ([self readDFUImageFile] == false) {
            [self notifyProcessCanceled];
            return;
        }
        // ツール同梱のイメージファイルのバージョンが、稼働中のファームウェアのバージョンより古い場合は処理を中止
        if ([self dfuImageIsAvailable] == false) {
            [self notifyProcessCanceled];
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
            [self notifyProcessCanceled];
            return;
        }
        // DFU処理開始
        [self invokeDFUProcess];
    }

    - (void)invokeDFUProcess {
        // 処理進捗画面（ダイアログ）をモーダルで表示
        [self bleDfuProcessingWindowWillOpen];
        // 処理進捗画面にDFU処理開始を通知
        [[self bleDfuProcessingWindow] commandDidStartDFUProcess:self maxProgressValue:(100 + DFU_WAITING_SEC_ESTIMATED)];
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
        // ステータスを初期化
        [self setBleDfuStatus:BLEDFU_ST_NONE];
        // 処理進捗画面を閉じる
        [[self bleDfuProcessingWindow] close];
        switch (modalResponse) {
            case NSModalResponseOK:
                [self notifyProcessTerminated:true];
                break;
            case NSModalResponseAbort:
                [self notifyProcessTerminated:false];
                break;
            case NSModalResponseCancel:
                // メッセージをポップアップ表示したのち、画面に制御を戻す
                [ToolPopupWindow critical:MSG_DFU_IMAGE_TRANSFER_CANCELED informativeText:nil];
                [self notifyProcessCanceled];
                break;
            default:
                break;
        }
    }

    - (void)bleDfuProcessingWindowNotifyCancel {
        // 処理進捗画面のCancelボタンがクリックされた場合
        [self notifyMessage:MSG_DFU_IMAGE_TRANSFER_CANCELED];
        // ステータスを更新（処理キャンセル）
        [self setBleDfuStatus:BLEDFU_ST_CANCELED];
    }

#pragma mark - Main process

    - (void)startDFUProcess {
        // 処理ステータスを更新
        [self setBleDfuStatus:BLEDFU_ST_UPLOAD_PROCESS];
        // 処理タイムアウト監視を開始
        [self startDFUTimeoutMonitor];
        // BLE DFU処理を開始
        [self doConnect];
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

    - (void)notifyCancelToProcessingWindow {
        dispatch_async([self mainQueue], ^{
            // 処理タイムアウト検知／バージョン更新判定フラグをリセット
            [self clearFlagsForProcess];
            // 処理進捗画面に対し、処理キャンセルの旨を通知する
            [[self bleDfuProcessingWindow] commandDidCancelDFUProcess];
        });
    }

#pragma mark - Process timeout monitor

    - (void)stopDFUTimeoutMonitor {
        // 処理タイムアウト監視を停止
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(DFUProcessDidTimeout) object:nil];
    }

    - (void)startDFUTimeoutMonitor {
        // 処理タイムアウト監視を事前停止
        [self stopDFUTimeoutMonitor];
        // 処理タイムアウト監視を開始（指定秒後にタイムアウト）
        [self performSelector:@selector(DFUProcessDidTimeout) withObject:nil afterDelay:TIMEOUT_SEC_DFU_PROCESS];
        // 処理タイムアウト検知フラグを設定
        [self setNeedTimeoutMonitor:true];
    }

    - (void)DFUProcessDidTimeout {
        // 処理タイムアウト検知フラグが設定されている場合
        if ([self needTimeoutMonitor]) {
            // 処理タイムアウトを検知したので、異常終了と判断
            [self notifyErrorMessage:MSG_DFU_PROCESS_TIMEOUT];
            // BLE接続を切断
            [self doDisconnectByError:true];
        }
    }

#pragma mark - DFU process

    - (void)doConnect {
        // BLE SMPサービスに接続 --> doRequestGetSlotInfoが呼び出される
        [[self toolBLESMPCommand] commandWillConnect];
    }

    - (void)doRequestGetSlotInfo {
        // DFU実行開始を通知
        [self notifyProgress:MSG_DFU_PROCESS_TRANSFER_IMAGE progressValue:0];
        // BLE経由でスロット照会を実行
        [[self toolBLESMPCommand] commandWillProcess:COMMAND_BLE_DFU_GET_SLOT_INFO request:nil forCommand:self];
    }

    - (void)doResponseGetSlotInfo:(bool)success response:(NSData *)response {
        // 処理失敗時は、画面に制御を戻す
        if (success == false) {
            [self notifyErrorMessage:MSG_DFU_SLOT_INFO_GET_FAILED];
            [self notifyErrorToProcessingWindow];
            return;
        }
        // スロット照会情報を参照し、チェックでNGの場合、BLE接続を切断
        if ([self checkSlotInfoWith:response] == false) {
            [self doDisconnectByError:true];
            return;
        }
        // 転送イメージ全体を取得
        [self setImageToUpload:[[NSData alloc] initWithBytes:mcumgr_app_image_bin() length:mcumgr_app_image_bin_size()]];
        // 転送済みバイト数を事前にクリア
        [[self toolBLESMPCommand] setImageBytesSent:0];
        // 転送処理に移行
        [self doRequestUploadImage];
    }

    - (void)doRequestUploadImage {
        // BLE経由でイメージ転送を実行
        [[self toolBLESMPCommand] commandWillProcess:COMMAND_BLE_DFU_UPLOAD_IMAGE request:[self imageToUpload] forCommand:self];
    }

    - (void)doResponseUploadImage:(bool)success response:(NSData *)response {
        // 処理失敗時は、画面に制御を戻す
        if (success == false) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_TRANSFER_FAILED];
            [self notifyErrorToProcessingWindow];
            return;
        }
        // 処理進捗画面でCancelボタンが押下された時は、転送処理を終了し、BLE接続を切断
        if ([self bleDfuStatus] == BLEDFU_ST_CANCELED) {
            [self doDisconnectByError:false];
            return;
        }
        // 転送結果情報を参照し、チェックでNGの場合、BLE接続を切断
        if ([self checkUploadResultInfoWith:response] == false) {
            [self doDisconnectByError:true];
            return;
        }
        // 転送結果情報の off 値を転送済みバイト数に設定
        size_t imageBytesSent = mcumgr_cbor_decode_result_info_off();
        [[self toolBLESMPCommand] setImageBytesSent:imageBytesSent];
        // 転送比率を計算
        size_t imageBytesTotal = [[self imageToUpload] length];
        int percentage = (int)imageBytesSent * 100 / (int)imageBytesTotal;
        [[ToolLogFile defaultLogger] debugWithFormat:@"DFU image sent %d bytes (%d%%)", imageBytesSent, percentage];
        // 転送状況を画面表示
        NSString *progressMessage = [NSString stringWithFormat:MSG_DFU_PROCESS_TRANSFER_IMAGE_FORMAT, percentage];
        [self notifyProgress:progressMessage progressValue:percentage];
        // イメージ全体が転送されたかどうかチェック
        if (imageBytesSent < imageBytesTotal) {
            // 処理進捗画面のCancelボタンを押下可能とする
            [self notifyCancelableToProcessingWindow:true];
            // 転送処理を続行
            [self doRequestUploadImage];
        } else {
            // 処理進捗画面のCancelボタンを押下不可とする
            [self notifyCancelableToProcessingWindow:false];
            // 反映要求に移行
            [self doRequestChangeImageUpdateMode];
        }
    }

    - (void)doRequestChangeImageUpdateMode {
        // SHA-256ハッシュデータをイメージから抽出
        NSData *hash = [[NSData alloc] initWithBytes:mcumgr_app_image_bin_hash_sha256() length:32];
        // BLE経由で反映要求を実行
        [[self toolBLESMPCommand] commandWillProcess:COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE request:hash forCommand:self];
    }

    - (void)doResponseChangeImageUpdateMode:(bool)success response:(NSData *)response {
        // 処理失敗時は、画面に制御を戻す
        if (success == false) {
            [self notifyErrorMessage:MSG_DFU_CHANGE_IMAGE_UPDATE_MODE_FAILED];
            [self notifyErrorToProcessingWindow];
            return;
        }
        // スロット照会情報を参照し、チェックでNGの場合、BLE接続を切断
        if ([self checkUploadedSlotInfoWith:response] == false) {
            [self doDisconnectByError:true];
            return;
        }
        // DFU転送成功を通知
        [self notifyMessage:MSG_DFU_IMAGE_TRANSFER_SUCCESS];
        // リセット要求処理に移行
        [self doRequestResetApplication];
    }

    - (void)doRequestResetApplication {
        // BLE経由でリセット要求を実行
        [[self toolBLESMPCommand] commandWillProcess:COMMAND_BLE_DFU_RESET_APPLICATION request:nil forCommand:self];
    }

    - (void)doResponseResetApplication:(bool)success response:(NSData *)response {
        // 処理失敗時は、画面に制御を戻す
        if (success == false) {
            [self notifyErrorMessage:MSG_DFU_RESET_APPLICATION_FAILED];
            [self notifyErrorToProcessingWindow];
            return;
        }
        // ステータスを更新（リセット要求済み）
        [self setBleDfuStatus:BLEDFU_ST_RESET_DONE];
        // nRF側が自動的にリセット --> 切断検知によりDFU反映待ち処理に移行
        [[ToolLogFile defaultLogger] debug:@"Requested to reset application"];
    }

    - (void) performDFUUpdateMonitor {
        // 反映待ち（リセットによるファームウェア再始動完了まで待機）
        for (int i = 0; i < DFU_WAITING_SEC_ESTIMATED; i++) {
            // 処理進捗画面に通知
            [self notifyProgress:MSG_DFU_PROCESS_WAITING_UPDATE progressValue:(100 + i)];
            [NSThread sleepForTimeInterval:1.0];
        }
        // 処理進捗画面に通知
        [self notifyProgress:MSG_DFU_PROCESS_CONFIRM_VERSION progressValue:(100 + DFU_WAITING_SEC_ESTIMATED)];
        // ステータスを更新（バージョン更新判定）
        [self setBleDfuStatus:BLEDFU_ST_CHECK_UPDATE_VERSION];
        // BLE経由でバージョン情報を取得 --> notifyFirmwareVersionが呼び出される
        [[self toolBLECommand] bleCommandWillProcess:COMMAND_BLE_GET_VERSION_INFO forCommand:self];
    }

    - (void)doDisconnectByError:(bool)b {
        [self setDisconnectByError:b];
        [[self toolBLESMPCommand] commandWillDisconnect];
    }

#pragma mark - Callback from BLE SMP transaction

    - (void)bleSmpCommandDidConnect {
        // スロット照会実行からスタート
        [self doRequestGetSlotInfo];
    }

    - (void)bleSmpCommandDidProcess:(Command)command success:(bool)success response:(NSData *)response forCommand:(id)ref {
        switch (command) {
            case COMMAND_BLE_DFU_GET_SLOT_INFO:
                [self doResponseGetSlotInfo:success response:response];
                break;
            case COMMAND_BLE_DFU_UPLOAD_IMAGE:
                [self doResponseUploadImage:success response:response];
                break;
            case COMMAND_BLE_DFU_CHANGE_IMAGE_UPDATE_MODE:
                [self doResponseChangeImageUpdateMode:success response:response];
                break;
            case COMMAND_BLE_DFU_RESET_APPLICATION:
                [self doResponseResetApplication:success response:response];
                break;
            default:
                break;
        }
    }

    - (void)bleSmpCommandDidDisconnectWithError:(NSError *)error {
        // リセット要求中に接続断が検知された場合
        if (error && [self bleDfuStatus] == BLEDFU_ST_RESET_DONE) {
            // 接続断検知の旨をログ出力
            [[ToolLogFile defaultLogger] debug:@"BLE disconnected by resetting application"];
            // ステータスを更新（DFU反映待ち）
            [self setBleDfuStatus:BLEDFU_ST_WAIT_FOR_BOOT];
            // DFU反映待ち処理に移行
            dispatch_async([self subQueue], ^{
                [self performDFUUpdateMonitor];
            });

        } else if ([self disconnectByError]) {
            // エラーとして画面に制御を戻す
            [self setDisconnectByError:false];
            [self notifyErrorToProcessingWindow];

        } else if ([self bleDfuStatus] == BLEDFU_ST_CANCELED) {
            // 転送キャンセルとして画面に制御を戻す
            [self notifyCancelToProcessingWindow];
        }
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
            NSString *informative = [NSString stringWithFormat:MSG_DFU_CURRENT_VERSION_OLD_FIRMWARE, update];
            [ToolPopupWindow critical:MSG_DFU_IMAGE_NOT_AVAILABLE informativeText:informative];
            return false;
        }
        // 更新バージョンを保持
        [self setUpdateVersionFromImage:update];
        return true;
    }

    - (bool)checkSlotInfoWith:(NSData *)response {
        // スロット照会情報を参照
        if ([self parseSmpSlotInfo:response] == false) {
            return false;
        }
        // SHA-256ハッシュデータをイメージから抽出
        NSData *imageHash = [[NSData alloc] initWithBytes:mcumgr_app_image_bin_hash_sha256() length:32];
        // 既に転送対象イメージが導入されている場合は、画面／ログにその旨を出力し、処理を中止
        if ([self checkSmpImageAlreadyConfirmed:response imageHash:imageHash]) {
            [self notifyErrorMessage:MSG_DFU_IMAGE_ALREADY_INSTALLED];
            return false;
        }
        return true;
    }

    - (bool)checkUploadedSlotInfoWith:(NSData *)response {
        // スロット照会情報を参照
        if ([self parseSmpSlotInfo:response] == false) {
            return false;
        }
        // スロット情報の代わりに rc が設定されている場合はエラー
        uint8_t rc = mcumgr_cbor_decode_result_info_rc();
        if (rc != 0) {
            NSString *message = [NSString stringWithFormat:MSG_DFU_IMAGE_INSTALL_FAILED_WITH_RC, rc];
            [self notifyErrorMessage:message];
            return false;
        }
        return true;
    }

    - (bool)parseSmpSlotInfo:(NSData *)responseData {
        // レスポンス（CBOR）を解析し、スロット照会情報を取得
        uint8_t *bytes = (uint8_t *)[responseData bytes];
        size_t size = [responseData length];
        if (mcumgr_cbor_decode_slot_info(bytes, size) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOR encode error: %s", log_debug_message()];
            return false;
        }
        return true;
    }

    - (bool)checkSmpImageAlreadyConfirmed:(NSData *)responseData imageHash:(NSData *)imageHash {
        // スロット照会情報から、スロット#0のハッシュを抽出
        uint8_t *hash0 = mcumgr_cbor_decode_slot_info_hash(0);
        NSData *hashData0 = [[NSData alloc] initWithBytes:hash0 length:32];
        // 既に転送対象イメージが導入されている場合は true
        if (mcumgr_cbor_decode_slot_info_active(0) && [hashData0 isEqualToData:imageHash]) {
            return true;
        }
        // 転送対象イメージが未導入と判定
        return false;
    }

    - (bool)checkUploadResultInfoWith:(NSData *)responseData {
        // レスポンス（CBOR）を解析し、転送結果情報を取得
        uint8_t *bytes = (uint8_t *)[responseData bytes];
        size_t size = [responseData length];
        if (mcumgr_cbor_decode_result_info(bytes, size) == false) {
            [[ToolLogFile defaultLogger] errorWithFormat:@"CBOR encode error: %s", log_debug_message()];
            return false;
        }
        // 転送結果情報の rc が設定されている場合はエラー
        uint8_t rc = mcumgr_cbor_decode_result_info_rc();
        if (rc != 0) {
            NSString *message = [NSString stringWithFormat:MSG_DFU_IMAGE_TRANSFER_FAILED_WITH_RC, rc];
            [self notifyErrorMessage:message];
            return false;
        }
        return true;
    }

#pragma mark - Private common methods

    - (void)clearFlagsForProcess {
        // 処理タイムアウト検知フラグをリセット
        [self setNeedTimeoutMonitor:false];
    }

    - (void)notifyProgress:(NSString *)message progressValue:(int)progress {
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に進捗を通知
            [[self bleDfuProcessingWindow] commandDidNotifyDFUProcess:message progressValue:progress];
        });
    }

    - (void)notifyCancelableToProcessingWindow:(bool)cancelable {
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面のCancelボタンを押下可／不可とする
            [[self bleDfuProcessingWindow] commandDidNotifyCancelable:cancelable];
        });
    }

    - (void)notifyMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] info:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyProcessTerminated:(bool)success {
        dispatch_async([self mainQueue], ^{
            // メイン画面に制御を戻す
            [[self toolAppCommand] commandDidProcess:COMMAND_BLE_DFU result:success message:nil];
        });
    }

    - (void)notifyProcessCanceled {
        dispatch_async([self mainQueue], ^{
            // メイン画面に制御を戻す
            [[self toolAppCommand] commandDidProcess:COMMAND_NONE result:true message:nil];
        });
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        // メイン画面にメッセージ文字列を表示する
        dispatch_async([self mainQueue], ^{
            [[[self toolAppCommand] delegate] notifyAppCommandMessage:message];
        });
    }

@end
