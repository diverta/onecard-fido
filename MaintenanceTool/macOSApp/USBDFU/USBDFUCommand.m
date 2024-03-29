//
//  USBDFUCommand.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/18.
//
#import "AppCommonMessage.h"
#import "AppHIDCommand.h"
#import "DFUCommand.h"
#import "FIDODefines.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"
#import "USBDFUCommand.h"
#import "USBDFUDefine.h"
#import "USBDFUImage.h"
#import "USBDFUTransferCommand.h"

// 以下は、BLE DFU機能と共通利用
#import "BLEDFUProcessingWindow.h"
#import "BLEDFUStartWindow.h"

@interface USBDFUCommand () <AppHIDCommandDelegate, USBDFUImageDelegate, USBDFUTransferCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    @property (nonatomic) USBDFUImage                  *usbDfuImage;
    @property (nonatomic) USBDFUTransferCommand        *transferCommand;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;
    // 非同期処理用のキュー（画面用）
    @property (nonatomic) dispatch_queue_t              mainQueue;
    // 画面の参照を保持
    @property (nonatomic) BLEDFUProcessingWindow       *dfuProcessingWindow;
    @property (nonatomic) BLEDFUStartWindow            *dfuStartWindow;

@end

@implementation USBDFUCommand

    - (id)init {
        return [self initWithDelegate:nil];
    }

    - (id)initWithDelegate:(id)delegate {
        self = [super init];
        if (self) {
            // 上位クラスの参照を保持
            [self setDelegate:delegate];
            // ヘルパークラスのインスタンスを生成
            [self setAppHIDCommand:[[AppHIDCommand alloc] initWithDelegate:self]];
            [self setUsbDfuImage:[[USBDFUImage alloc] initWithDelegate:self]];
            [self setTransferCommand:[[USBDFUTransferCommand alloc] initWithDelegate:self]];
            // 画面のインスタンスを生成
            [self setDfuStartWindow:[[BLEDFUStartWindow alloc] initWithWindowNibName:@"BLEDFUStartWindow"]];
            [self setDfuProcessingWindow:[[BLEDFUProcessingWindow alloc] initWithWindowNibName:@"BLEDFUProcessingWindow"]];
            // メインスレッドにバインドされるデフォルトキューを取得
            [self setMainQueue:dispatch_get_main_queue()];
            // パラメーターを初期化
            [self setCommandParameter:[[DFUCommandParameter alloc] init]];
            [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        }
        return self;
    }

    - (bool)isUSBHIDConnected {
        // USBポートに接続されていない場合はfalse
        return [[self appHIDCommand] checkUSBHIDConnection];
    }

#pragma mark - Command/subcommand process

    - (void)usbDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow {
        // 親画面の参照を保持
        [self setParentWindow:parentWindow];
        // 処理開始画面に親画面参照をセット
        [[self dfuStartWindow] setParentWindow:parentWindow];
        [[self dfuProcessingWindow] setParentWindow:parentWindow];
        // 事前にHID経由でバージョン情報を取得
        [self doRequestHIDGetVersionInfo];
    }

    - (void)usbDfuProcessDidCompleted:(bool)result message:(NSString *)message {
        // 上位クラスに制御を戻す
        [[self delegate] notifyCommandTerminated:COMMAND_USB_DFU success:result message:message];
    }

#pragma mark - Version checking process

    - (void)doRequestHIDGetVersionInfo {
        // ステータスを更新（現在バージョン照会）
        [[self commandParameter] setDfuStatus:DFU_ST_GET_CURRENT_VERSION];
        // HID経由でFlash ROM情報を取得（コマンド 0xC3 を実行、メッセージ無し）
        uint8_t cmd = MNT_COMMAND_BASE | 0x80;
        [[self appHIDCommand] doRequestCommand:COMMAND_HID_GET_VERSION_INFO withCMD:cmd withData:[ToolCommonFunc commandDataForGetVersionInfo]];
    }

    - (void)doResponseHIDGetVersionInfo:(NSData *)versionInfoResponse {
        // 現在バージョン照会の場合（処理開始画面の表示前）
        if ([[self commandParameter] dfuStatus] == DFU_ST_GET_CURRENT_VERSION) {
            [self notifyFirmwareVersionForStart:true response:versionInfoResponse];
        }
    }

    - (void)notifyFirmwareVersionForStart:(bool)success response:(NSData *)response {
        if (success == false || response == nil || [response length] < 2) {
            // エラーが発生した場合は、メッセージをログ出力／ポップアップ表示したのち、画面に制御を戻す
            [[ToolLogFile defaultLogger] error:MSG_DFU_VERSION_INFO_GET_FAILED];
            [[ToolPopupWindow defaultWindow] critical:MSG_DFU_VERSION_INFO_GET_FAILED informativeText:nil withObject:self forSelector:@selector(notifyProcessCanceled) parentWindow:[self parentWindow]];
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
        [[self commandParameter] setCurrentVersion:array[1]];
        [[self commandParameter] setCurrentBoardname:array[2]];
    }

    - (void)resumeDfuProcessStart {
        // 基板名に対応するファームウェア更新イメージファイルから、バイナリーイメージを読込
        if ([[self usbDfuImage] readDFUImageFile:[self commandParameter]] == false) {
            [self notifyProcessCanceled];
            return;
        }
        // ツール同梱のイメージファイルのバージョンが、稼働中のファームウェアのバージョンより古い場合は処理を中止
        if ([[self usbDfuImage] dfuImageIsAvailable:[self commandParameter]] == false) {
            [self notifyProcessCanceled];
            return;
        }
        // 処理開始画面を表示
        [self dfuStartWindowWillOpen];
    }

#pragma mark - Interface for DFUStartWindow

    - (void)dfuStartWindowWillOpen {
        NSWindow *dialog = [[self dfuStartWindow] window];
        USBDFUCommand * __weak weakSelf = self;
        [[[self dfuStartWindow] parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuStartWindowDidClose:self modalResponse:response];
        }];
        // バージョン情報を、ダイアログの該当欄に設定
        [[self dfuStartWindow] setWindowParameter:self
            currentVersion:[[self commandParameter] currentVersion] updateVersion:[[self commandParameter] updateVersionFromImage]];
    }

    - (void)dfuStartWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [[self dfuStartWindow] close];
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
        [self dfuProcessingWindowWillOpen];
        // 処理進捗画面にDFU処理開始を通知
        [[self dfuProcessingWindow] commandDidStartDFUProcess:self maxProgressValue:(100 + USBDFU_WAITING_SEC_ESTIMATED)];
        [[self dfuProcessingWindow] commandDidNotifyDFUProcess:MSG_DFU_PROCESS_TRANSFER_PREPARE progressValue:0];
        // DFU処理を開始
        [self startDFUProcess];
    }

#pragma mark - Interface for DFUProcessingWindow

    - (void)dfuProcessingWindowWillOpen {
        NSWindow *dialog = [[self dfuProcessingWindow] window];
        USBDFUCommand * __weak weakSelf = self;
        [[[self dfuProcessingWindow] parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf dfuProcessingWindowDidClose:self modalResponse:response];
        }];
    }

    - (void)dfuProcessingWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // ステータスを初期化
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        // 処理進捗画面を閉じる
        [[self dfuProcessingWindow] close];
        switch (modalResponse) {
            case NSModalResponseOK:
                [self notifyProcessTerminated:true];
                break;
            case NSModalResponseAbort:
                [self notifyProcessTerminated:false];
                break;
            case NSModalResponseCancel:
                // メッセージをポップアップ表示したのち、画面に制御を戻す
                [[ToolPopupWindow defaultWindow] critical:MSG_DFU_IMAGE_TRANSFER_CANCELED informativeText:nil withObject:self forSelector:@selector(notifyProcessCanceled) parentWindow:[self parentWindow]];
                break;
            default:
                break;
        }
    }

    - (void)notifyProgress:(NSString *)message progressValue:(int)progressValue {
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に進捗を通知
            [[self dfuProcessingWindow] commandDidNotifyDFUProcess:message progressValue:progressValue];
        });
    }

#pragma mark - Main process

    - (void)startDFUProcess {
        // 処理ステータスをクリア
        [[self commandParameter] setDfuStatus:DFU_ST_NONE];
        // 処理タイムアウト監視を開始
        [self startDFUTimeoutMonitor];
        // 転送処理を開始
        [[self transferCommand] invokeTransferWithParamRef:[self commandParameter]];
        // メイン画面に開始メッセージを出力
        dispatch_async([self mainQueue], ^{
            [[self delegate] notifyCommandStartedWithCommand:COMMAND_USB_DFU];
        });
    }

#pragma mark - Process timeout monitor

    - (void)stopDFUTimeoutMonitor {
        dispatch_async([self mainQueue], ^{
            // 処理タイムアウト監視を停止
            [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(DFUProcessDidTimeout) object:nil];
        });
    }

    - (void)startDFUTimeoutMonitor {
        dispatch_async([self mainQueue], ^{
            // 処理タイムアウト監視を事前停止
            [self stopDFUTimeoutMonitor];
            // 処理タイムアウト監視を開始（指定秒後にタイムアウト）
            [self performSelector:@selector(DFUProcessDidTimeout) withObject:nil afterDelay:TIMEOUT_SEC_DFU_PROCESS];
        });
    }

    - (void)DFUProcessDidTimeout {
        // 処理タイムアウトを検知したので、異常終了と判断
        [self notifyErrorMessage:MSG_DFU_PROCESS_TIMEOUT];
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に対し、処理失敗の旨を通知する
            [[self dfuProcessingWindow] commandDidTerminateDFUProcess:false];
        });
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command CMD:(uint8_t)cmd response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        if (command == COMMAND_HID_GET_VERSION_INFO) {
            if (success == false) {
                // 即時で上位クラスに制御を戻す
                [self usbDfuProcessDidCompleted:false message:errorMessage];
                return;
            }
            // 処理を続行
            [self doResponseHIDGetVersionInfo:response];
        }
    }

#pragma mark - Call back from USBDFUImage

    - (void)notifyCriticalErrorMessage:(NSString *)errorMessage informative:(NSString *)informativeMessage {
        [[ToolPopupWindow defaultWindow] critical:errorMessage informativeText:informativeMessage
                                       withObject:nil forSelector:nil parentWindow:[self parentWindow]];
    }

#pragma mark - Call back from USBDFUTransferCommand

    - (void)transferCommandDidTerminate:(bool)success {
        // 処理タイムアウト監視を停止
        [self stopDFUTimeoutMonitor];
        dispatch_async([self mainQueue], ^{
            // 処理進捗画面に対し、処理成功／失敗の旨を通知する
            [[self dfuProcessingWindow] commandDidTerminateDFUProcess:success];
        });
    }

#pragma mark - Private common methods

    - (void)notifyInfoMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] info:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyErrorMessage:(NSString *)message {
        [[ToolLogFile defaultLogger] error:message];
        [self notifyToolCommandMessage:message];
    }

    - (void)notifyToolCommandMessage:(NSString *)message {
        // メイン画面にメッセージ文字列を表示する
        dispatch_async([self mainQueue], ^{
            [[self delegate] notifyMessage:message];
        });
    }

    - (void)notifyProcessTerminated:(bool)success {
        dispatch_async([self mainQueue], ^{
            // メイン画面に制御を戻す
            [[self delegate] notifyCommandTerminated:COMMAND_USB_DFU success:success message:nil];
        });
    }

    - (void)notifyProcessCanceled {
        dispatch_async([self mainQueue], ^{
            // メイン画面に制御を戻す
            [[self delegate] notifyCommandTerminated:COMMAND_NONE success:true message:nil];
        });
    }

@end
