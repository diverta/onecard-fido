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
#import "ToolLogFile.h"
#import "ToolPopupWindow.h"
#import "USBDFUCommand.h"

@interface USBDFUCommand () <AppHIDCommandDelegate>

    // 上位クラスの参照を保持
    @property (nonatomic, weak) id                      delegate;
    // ヘルパークラスの参照を保持
    @property (nonatomic) AppHIDCommand                *appHIDCommand;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // DFU処理のパラメーターを保持
    @property (nonatomic) DFUCommandParameter          *commandParameter;
    // 非同期処理用のキュー（画面用／DFU処理用）
    @property (nonatomic) dispatch_queue_t              mainQueue;
    @property (nonatomic) dispatch_queue_t              subQueue;

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
            // メインスレッド／サブスレッドにバインドされるデフォルトキューを取得
            [self setMainQueue:dispatch_get_main_queue()];
            [self setSubQueue:dispatch_queue_create("jp.co.diverta.fido.maintenancetool.usbdfu", DISPATCH_QUEUE_SERIAL)];
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
        [[self appHIDCommand] doRequestCommand:COMMAND_HID_GET_VERSION_INFO withCMD:HID_CMD_GET_VERSION_INFO withData:nil];
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
        // TODO: 仮の実装です。
        [[self delegate] notifyCommandStartedWithCommand:COMMAND_USB_DFU];
        [self usbDfuProcessDidCompleted:false message:MSG_CMDTST_MENU_NOT_SUPPORTED];
    }

#pragma mark - Call back from AppHIDCommand

    - (void)didDetectConnect {
    }

    - (void)didDetectRemoval {
    }

    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage {
        // 即時で上位クラスに制御を戻す
        if (success == false) {
            [self usbDfuProcessDidCompleted:false message:errorMessage];
            return;
        }
        // 実行コマンドにより処理分岐
        switch (command) {
            case COMMAND_HID_GET_VERSION_INFO:
                [self doResponseHIDGetVersionInfo:response];
                break;
            default:
                // 正しくレスポンスされなかったと判断し、上位クラスに制御を戻す
                [self usbDfuProcessDidCompleted:false message:MSG_OCCUR_UNKNOWN_ERROR];
                break;
        }
    }

#pragma mark - Private common methods

    - (void)notifyProcessCanceled {
        dispatch_async([self mainQueue], ^{
            // メイン画面に制御を戻す
            [[self delegate] notifyCommandTerminated:COMMAND_NONE success:true message:nil];
        });
    }

@end
