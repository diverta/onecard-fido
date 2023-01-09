//
//  PIVPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "PIVPreferenceWindow.h"
#import "ToolCommon.h"
#import "ToolCommonFunc.h"
#import "ToolFilePanel.h"
#import "ToolInfoWindow.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"
#import "ToolLogFile.h"

@interface PIVPreferenceWindow () <ToolFilePanelDelegate>

    // ファイル選択用のテキストボックス、ボタン
    @property (assign) IBOutlet NSView              *windowView;
    @property (assign) IBOutlet NSTabView           *tabView;
    @property (assign) IBOutlet NSButton            *buttonClose;
    @property (assign) IBOutlet NSButton            *buttonFirmwareReset;
    @property (assign) IBOutlet NSButton            *buttonPivStatus;
    @property (assign) IBOutlet NSButton            *buttonInitialSetting;
    @property (assign) IBOutlet NSButton            *buttonClearSetting;

    @property (assign) IBOutlet NSTabViewItem       *tabPkeyCertManagement;
    @property (assign) IBOutlet NSTextField         *textPkeyFilePath1;
    @property (assign) IBOutlet NSTextField         *textCertFilePath1;
    @property (assign) IBOutlet NSButton            *buttonPkeyFilePath1;
    @property (assign) IBOutlet NSButton            *buttonCertFilePath1;
    @property (assign) IBOutlet NSTextField         *textPkeyFilePath2;
    @property (assign) IBOutlet NSTextField         *textCertFilePath2;
    @property (assign) IBOutlet NSButton            *buttonPkeyFilePath2;
    @property (assign) IBOutlet NSButton            *buttonCertFilePath2;
    @property (assign) IBOutlet NSTextField         *textPkeyFilePath3;
    @property (assign) IBOutlet NSTextField         *textCertFilePath3;
    @property (assign) IBOutlet NSButton            *buttonPkeyFilePath3;
    @property (assign) IBOutlet NSButton            *buttonCertFilePath3;
    @property (assign) IBOutlet NSTextField         *fieldPin1;
    @property (assign) IBOutlet NSTextField         *fieldPin2;
    @property (assign) IBOutlet NSButton            *buttonInstallPkeyCert;

    @property (assign) IBOutlet NSTabViewItem       *tabPinManagement;
    @property (assign) IBOutlet NSButton            *buttonPinCommand1;
    @property (assign) IBOutlet NSButton            *buttonPinCommand2;
    @property (assign) IBOutlet NSButton            *buttonPinCommand3;
    @property (assign) IBOutlet NSTextField         *labelCurPin;
    @property (assign) IBOutlet NSTextField         *labelNewPin;
    @property (assign) IBOutlet NSTextField         *labelNewPinConf;
    @property (assign) IBOutlet NSTextField         *fieldCurPin;
    @property (assign) IBOutlet NSTextField         *fieldNewPin;
    @property (assign) IBOutlet NSTextField         *fieldNewPinConf;
    @property (assign) IBOutlet NSButton            *buttonPerformPinCommand;

    // 親画面を保持
    @property (nonatomic, weak) NSWindow            *parentWindow;
    // PIV機能処理クラスの参照を保持
    @property (nonatomic, weak) ToolPIVCommand      *toolPIVCommand;
    @property (nonatomic) ToolFilePanel             *toolFilePanel;
    // ラジオボタンで選択中の情報を保持
    @property (nonatomic) Command                    selectedPinCommand;

@end

@implementation PIVPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // PIN番号管理タブ内の入力項目を初期化
        [[self tabView] selectTabViewItem:[self tabPinManagement]];
        [self initTabPinManagement];
        // 鍵・証明書管理タブ内の入力項目を初期化（このタブが選択状態になります）
        [[self tabView] selectTabViewItem:[self tabPkeyCertManagement]];
        [self initTabPkeyCertManagement];
    }

    - (void)initTabPkeyCertManagement {
        // テキストボックスの初期化
        [self initTabPkeyCertPathFields];
        [self initTabPkeyCertPinFields];
    }

    - (void)initTabPkeyCertPathFields {
        // ファイルパスのテキストボックスを初期化
        [[self textPkeyFilePath1] setStringValue:@""];
        [[self textCertFilePath1] setStringValue:@""];
        [[self textPkeyFilePath1] setToolTip:@""];
        [[self textCertFilePath1] setToolTip:@""];
        [[self textPkeyFilePath2] setStringValue:@""];
        [[self textCertFilePath2] setStringValue:@""];
        [[self textPkeyFilePath2] setToolTip:@""];
        [[self textCertFilePath2] setToolTip:@""];
        [[self textPkeyFilePath3] setStringValue:@""];
        [[self textCertFilePath3] setStringValue:@""];
        [[self textPkeyFilePath3] setToolTip:@""];
        [[self textCertFilePath3] setToolTip:@""];
    }

    - (void)initTabPkeyCertPinFields {
        // PIN番号のテキストボックスを初期化
        [[self fieldPin1] setStringValue:@""];
        [[self fieldPin2] setStringValue:@""];
        // テキストボックスのカーソルを先頭の項目に配置
        [[self fieldPin1] becomeFirstResponder];
    }

    - (void)initTabPinManagement {
        // ラジオボタンの初期化
        [self initButtonPinCommandsWithDefault:[self buttonPinCommand1]];
        // テキストボックスの初期化
        [self initTabPinManagementPinFields];
    }

    - (void)initTabPinManagementPinFields {
        // PIN番号のテキストボックスを初期化
        [[self fieldCurPin] setStringValue:@""];
        [[self fieldNewPin] setStringValue:@""];
        [[self fieldNewPinConf] setStringValue:@""];
        // テキストボックスのカーソルを先頭の項目に配置
        [[self fieldCurPin] becomeFirstResponder];
    }

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonClose] setEnabled:enabled];
        [[self buttonFirmwareReset] setEnabled:enabled];
        [[self buttonPivStatus] setEnabled:enabled];
        [[self buttonInitialSetting] setEnabled:enabled];
        [[self buttonClearSetting] setEnabled:enabled];
        // 現在選択中のタブ内も同様に制御を行う
        NSTabViewItem *item = [[self tabView] selectedTabViewItem];
        if (item == [self tabPkeyCertManagement]) {
            [self enableButtonsInTabPkeyCertManagement:enabled];
        }
        if (item == [self tabPinManagement]) {
            [self enableButtonsInTabPinManagement:enabled];
        }
    }

    - (void)enableButtonsInTabPkeyCertManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self textPkeyFilePath1] setEnabled:enabled];
        [[self textCertFilePath1] setEnabled:enabled];
        [[self buttonPkeyFilePath1] setEnabled:enabled];
        [[self buttonCertFilePath1] setEnabled:enabled];
        [[self textPkeyFilePath2] setEnabled:enabled];
        [[self textCertFilePath2] setEnabled:enabled];
        [[self buttonPkeyFilePath2] setEnabled:enabled];
        [[self buttonCertFilePath2] setEnabled:enabled];
        [[self textPkeyFilePath3] setEnabled:enabled];
        [[self textCertFilePath3] setEnabled:enabled];
        [[self buttonPkeyFilePath3] setEnabled:enabled];
        [[self buttonCertFilePath3] setEnabled:enabled];
        [[self fieldPin1] setEnabled:enabled];
        [[self fieldPin2] setEnabled:enabled];
        [[self buttonInstallPkeyCert] setEnabled:enabled];
    }

    - (void)enableButtonsInTabPinManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonPinCommand1] setEnabled:enabled];
        [[self buttonPinCommand2] setEnabled:enabled];
        [[self buttonPinCommand3] setEnabled:enabled];
        [[self fieldCurPin] setEnabled:enabled];
        [[self fieldNewPin] setEnabled:enabled];
        [[self fieldNewPinConf] setEnabled:enabled];
        [[self buttonPerformPinCommand] setEnabled:enabled];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

    - (IBAction)buttonPivStatusDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // PIV設定情報取得
        [self enableButtons:false];
        [self commandWillPerformPIVProcess:COMMAND_CCID_PIV_STATUS withParameter:nil];
    }

    - (IBAction)buttonInitialSettingDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        NSString *msg = [[NSString alloc] initWithFormat:MSG_FORMAT_WILL_PROCESS, MSG_PIV_INITIAL_SETTING];
        [[ToolPopupWindow defaultWindow] informationalPrompt:msg informativeText:MSG_PROMPT_PIV_INITIAL_SETTING
                                                  withObject:self forSelector:@selector(initialSettingCommandPromptDone) parentWindow:[self window]];
    }

    - (void)initialSettingCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // CHUID設定機能を実行
        [self enableButtons:false];
        [self commandWillPerformPIVProcess:COMMAND_CCID_PIV_SET_CHUID withParameter:nil];
    }

    - (IBAction)buttonClearSettingDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        NSString *msg = [[NSString alloc] initWithFormat:MSG_FORMAT_WILL_PROCESS, MSG_PIV_CLEAR_SETTING];
        [[ToolPopupWindow defaultWindow] criticalPrompt:msg informativeText:MSG_PROMPT_PIV_CLEAR_SETTING
                                             withObject:self forSelector:@selector(clearSettingCommandPromptDone) parentWindow:[self window]];
    }

    - (void)clearSettingCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // PIVリセット機能を実行
        [self enableButtons:false];
        [self commandWillPerformPIVProcess:COMMAND_CCID_PIV_RESET withParameter:nil];
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonFirmwareResetDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 認証器のファームウェアを再起動
        [self enableButtons:false];
        [self commandWillPerformPIVProcess:COMMAND_HID_FIRMWARE_RESET withParameter:nil];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if ([[self toolPIVCommand] checkUSBHIDConnection]) {
            return true;
        }
        // エラーメッセージをポップアップ表示
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil withObject:nil forSelector:nil parentWindow:[self window]];
        return false;
    }

#pragma mark - For PIVPreferenceWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // ToolPIVCommandの参照を保持
        [self setToolPIVCommand:ref];
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        PIVPreferenceWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithSender:ref modalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        if ([sender isMemberOfClass:[ToolPIVCommand class]]) {
            // ToolPIVCommand経由でメイン画面に制御を戻す
            ToolPIVCommand *command = (ToolPIVCommand *)sender;
            [command commandDidClosePreferenceWindow];
        }
    }

#pragma mark - 鍵・証明書管理タブ関連

    - (IBAction)buttonPkeyFilePathDidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }

    - (IBAction)buttonCertFilePathDidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH];
    }

    - (IBAction)buttonInstallPkeyCertDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 入力欄の内容をチェック
        if ([self checkForInstallPkeyCert:sender] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_INSTALL_PIV_PKEY_CERT informativeText:MSG_PROMPT_INSTL_SKEY_CERT
                                                  withObject:self forSelector:@selector(installPkeyCertCommandPromptDone) parentWindow:[self window]];
    }

    - (void)installPkeyCertCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 画面入力内容をパラメーターに格納
        ToolPIVParameter *parameter = [[ToolPIVParameter alloc] init];
        [parameter setPkeyPemPath1:[[self textPkeyFilePath1] toolTip]];
        [parameter setCertPemPath1:[[self textCertFilePath1] toolTip]];
        [parameter setPkeyPemPath2:[[self textPkeyFilePath2] toolTip]];
        [parameter setCertPemPath2:[[self textCertFilePath2] toolTip]];
        [parameter setPkeyPemPath3:[[self textPkeyFilePath3] toolTip]];
        [parameter setCertPemPath3:[[self textCertFilePath3] toolTip]];
        [parameter setAuthPin:[[self fieldPin2] stringValue]];
        // PIV認証用の鍵・証明書インストール
        [self commandWillPerformPIVProcess:COMMAND_CCID_PIV_IMPORT_KEY withParameter:parameter];
    }

    - (void)panelWillSelectPath:(id)sender withPrompt:(NSString *)prompt {
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:prompt withFileTypes:@[@"pem"]];
    }

    - (void)panelDidSelectPath:(id)sender filePath:(NSString *)filePath modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse != NSFileHandlingPanelOKButton) {
            return;
        }
        [self setFieldPath:sender filePath:filePath];
        [[self textPkeyFilePath1] moveToEndOfDocument:sender];
        
    }

    - (void)setFieldPath:(id)sender filePath:(NSString *)filePath {
        // ファイルパスをディレクトリー、ファイル名に分割し、テキストボックスにはファイル名のみ設定
        NSString *fileName = [filePath lastPathComponent];
        if (sender == [self buttonPkeyFilePath1]) {
            [[self textPkeyFilePath1] setStringValue:fileName];
            [[self textPkeyFilePath1] setToolTip:filePath];
        }
        if (sender == [self buttonCertFilePath1]) {
            [[self textCertFilePath1] setStringValue:fileName];
            [[self textCertFilePath1] setToolTip:filePath];
        }
        if (sender == [self buttonPkeyFilePath2]) {
            [[self textPkeyFilePath2] setStringValue:fileName];
            [[self textPkeyFilePath2] setToolTip:filePath];
        }
        if (sender == [self buttonCertFilePath2]) {
            [[self textCertFilePath2] setStringValue:fileName];
            [[self textCertFilePath2] setToolTip:filePath];
        }
        if (sender == [self buttonPkeyFilePath3]) {
            [[self textPkeyFilePath3] setStringValue:fileName];
            [[self textPkeyFilePath3] setToolTip:filePath];
        }
        if (sender == [self buttonCertFilePath3]) {
            [[self textCertFilePath3] setStringValue:fileName];
            [[self textCertFilePath3] setToolTip:filePath];
        }
    }

#pragma mark - PIN番号管理タブ関連

    - (IBAction)buttonPinCommandSelected:(id)sender {
        [self getSelectedPinCommandValue:sender];
    }

    - (IBAction)buttonPerformPinCommandDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // ラジオボタンから実行コマンド種別を取得
        Command command = [self selectedPinCommand];
        // 入力欄の内容をチェック
        if ([self checkForPerformPinCommand:sender withCommand:command] == false) {
            return;
        }
        // 処理名称、詳細を設定
        NSString *name = [self functionNameOfCommand:command];
        NSString *desc = [self descriptionOfCommand:command];
        // 事前に確認ダイアログを表示
        NSString *msg = [[NSString alloc] initWithFormat:MSG_FORMAT_WILL_PROCESS, name];
        NSString *informative = [[NSString alloc] initWithFormat:MSG_FORMAT_PROCESS_INFORMATIVE, desc];
        [[ToolPopupWindow defaultWindow] criticalPrompt:msg informativeText:informative
                                             withObject:self forSelector:@selector(performPinCommandPromptDone) parentWindow:[self window]];
    }

    - (void)performPinCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // ラジオボタンから実行コマンド種別を取得
        Command command = [self selectedPinCommand];
        // 画面入力内容をパラメーターに格納
        ToolPIVParameter *parameter = [[ToolPIVParameter alloc] init];
        [parameter setCurrentPin:[[self fieldCurPin] stringValue]];
        [parameter setRenewalPin:[[self fieldNewPinConf] stringValue]];
        // PIN番号管理コマンドを実行（ラジオボタンから実行コマンドを取得）
        [self commandWillPerformPIVProcess:command withParameter:parameter];
    }

    - (void)initButtonPinCommandsWithDefault:(NSButton *)defaultButton {
        // 「実行する機能」のラジオボタン「PIN番号を変更」を選択状態にする
        [defaultButton setState:NSControlStateValueOn];
        [self getSelectedPinCommandValue:defaultButton];
    }

    - (void)getSelectedPinCommandValue:(NSButton *)button {
        // ラジオボタンの選択状態に応じ、入力欄のキャプションも変更する
        if (button == [self buttonPinCommand1]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_CHANGE_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PIN];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PIN_FOR_CONFIRM];
        } else if (button == [self buttonPinCommand2]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_CHANGE_PUK];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PUK];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PUK];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PUK_FOR_CONFIRM];
        } else if (button == [self buttonPinCommand3]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_UNBLOCK_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PUK];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PIN];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PIN_FOR_CONFIRM];
        } else {
            [self setSelectedPinCommand:COMMAND_NONE];
        }
    }

#pragma mark - 入力チェック関連

    - (bool)checkForInstallPkeyCert:(id)sender {
        // 入力欄のチェック
        if ([self checkPathEntry:[self textPkeyFilePath1] messageIfError:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textCertFilePath1] messageIfError:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textPkeyFilePath2] messageIfError:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH_2] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textCertFilePath2] messageIfError:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH_2] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textPkeyFilePath3] messageIfError:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH_3] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textCertFilePath3] messageIfError:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH_3] == false) {
            return false;
        }
        if ([self checkPinNumber:[self fieldPin1] withName:MSG_LABEL_CURRENT_PIN] == false) {
            return false;
        }
        if ([self checkPinNumber:[self fieldPin2] withName:MSG_LABEL_CURRENT_PIN_FOR_CONFIRM] == false) {
            return false;
        }
        // 確認用PINコードのチェック
        if ([self checkPinConfirmFor:[self fieldPin2] withSource:[self fieldPin1]
                            withName:MSG_LABEL_CURRENT_PIN_FOR_CONFIRM] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPathEntry:(NSTextField *)field messageIfError:(NSString *)message {
        // 入力項目が正しく指定されていない場合は終了
        if ([ToolCommonFunc checkMustEntry:field informativeText:message onWindow:[self window]] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolCommonFunc checkFileExist:field forPath:[field toolTip] informativeText:message onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

    - (bool) checkPinNumber:(NSTextField *)field withName:(NSString *)name {
        // 長さチェック
        NSString *msg1 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_DIGIT, name];
        if ([ToolCommon checkEntrySize:field minSize:PIV_PIN_CODE_SIZE_MIN maxSize:PIV_PIN_CODE_SIZE_MAX
                       informativeText:msg1 onWindow:[self window]] == false) {
            return false;
        }
        // 数字チェック
        NSString *msg2 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_NUM, name];
        if ([ToolCommon checkIsNumeric:field informativeText:msg2 onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPinConfirmFor:(NSTextField *)dest withSource:(NSTextField *)source withName:(NSString *)name {
        NSString *msg = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_CONFIRM, name];
        return [ToolCommon compareEntry:dest srcField:source informativeText:msg onWindow:[self window]];
    }

    - (bool)checkForPerformPinCommand:(id)sender withCommand:(Command)command {
        // 入力欄のチェック
        if ([self checkPinNumbersForPinCommand:command] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPinNumbersForPinCommand:(Command)command {
        // ポップアップ表示される文言を設定
        NSString *msgCurPin = nil;
        NSString *msgNewPin = nil;
        NSString *msgNewPinConf = nil;
        switch (command) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                msgCurPin = MSG_LABEL_CURRENT_PIN;
                msgNewPin = MSG_LABEL_NEW_PIN;
                msgNewPinConf = MSG_LABEL_NEW_PIN_FOR_CONFIRM;
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                msgCurPin = MSG_LABEL_CURRENT_PUK;
                msgNewPin = MSG_LABEL_NEW_PUK;
                msgNewPinConf = MSG_LABEL_NEW_PUK_FOR_CONFIRM;
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                msgCurPin = MSG_LABEL_CURRENT_PUK;
                msgNewPin = MSG_LABEL_NEW_PIN;
                msgNewPinConf = MSG_LABEL_NEW_PIN_FOR_CONFIRM;
                break;
            default:
                break;
        }
        // 入力欄のチェック
        if ([self checkPinNumber:[self fieldCurPin] withName:msgCurPin] == false) {
            return false;
        }
        if ([self checkPinNumber:[self fieldNewPin] withName:msgNewPin] == false) {
            return false;
        }
        if ([self checkPinNumber:[self fieldNewPinConf] withName:msgNewPinConf] == false) {
            return false;
        }
        // 確認用PINコードのチェック
        if ([self checkPinConfirmFor:[self fieldNewPinConf] withSource:[self fieldNewPin] withName:msgNewPinConf] == false) {
            return false;
        }
        return true;
    }

    - (NSString *)functionNameOfCommand:(Command)command {
        // 処理名称を設定
        NSString *name = nil;
        switch (command) {
            case COMMAND_CCID_PIV_STATUS:
                name = MSG_PIV_STATUS;
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                name = MSG_PIV_INITIAL_SETTING;
                break;
            case COMMAND_CCID_PIV_RESET:
                name = MSG_PIV_CLEAR_SETTING;
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
                name = MSG_PIV_INSTALL_PKEY_CERT;
                break;
            case COMMAND_CCID_PIV_CHANGE_PIN:
                name = MSG_PIV_CHANGE_PIN_NUMBER;
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                name = MSG_PIV_CHANGE_PUK_NUMBER;
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                name = MSG_PIV_RESET_PIN_NUMBER;
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                name = PROCESS_NAME_FIRMWARE_RESET;
                break;
            default:
                break;
        }
        return name;
    }

    - (NSString *)descriptionOfCommand:(Command)command {
        // 処理詳細を設定
        NSString *name = nil;
        switch (command) {
            case COMMAND_CCID_PIV_CHANGE_PIN:
                name = MSG_DESC_PIV_CHANGE_PIN_NUMBER;
                break;
            case COMMAND_CCID_PIV_CHANGE_PUK:
                name = MSG_DESC_PIV_CHANGE_PUK_NUMBER;
                break;
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                name = MSG_DESC_PIV_RESET_PIN_NUMBER;
                break;
            default:
                break;
        }
        return name;
    }

#pragma mark - For ToolPIVCommand functions

    - (void)commandWillPerformPIVProcess:(Command)command withParameter:(ToolPIVParameter *)parameter {
        // 進捗画面を表示
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        // PIV機能を実行
        [[self toolPIVCommand] commandWillPerformPIVProcess:command withParameter:parameter];
    }

    - (void)toolPIVCommandDidProcess:(Command)command withResult:(bool)result withErrorMessage:(NSString *)errorMessage {
        if (command == COMMAND_CCID_PIV_STATUS && result) {
            // 進捗画面を閉じる
            [[ToolProcessingWindow defaultWindow] windowWillClose:NSModalResponseCancel withMessage:nil withInformative:nil];
            // PIV設定情報を、情報表示画面に表示
            [[ToolInfoWindow defaultWindow] windowWillOpenWithCommandRef:[self toolPIVCommand]
                withParentWindow:[self window] titleString:PROCESS_NAME_CCID_PIV_STATUS
                infoString:[[self toolPIVCommand] getPIVSettingDescriptionString]];
            // 画面項目を使用可とする
            [self enableButtons:true];
            return;
        }
        // ポップアップ表示させるメッセージを編集
        NSString *message = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, [self functionNameOfCommand:command],
                             result ? MSG_SUCCESS:MSG_FAILURE];
        // 進捗画面を閉じ、処理終了メッセージをポップアップ表示
        if (result) {
            [[ToolProcessingWindow defaultWindow] windowWillClose:NSModalResponseOK withMessage:message withInformative:nil];
        } else {
            [[ToolProcessingWindow defaultWindow] windowWillClose:NSModalResponseAbort withMessage:message withInformative:errorMessage];
        }
        // 画面項目を使用可とする
        [self clearEntry:command withResult:result];
        [self enableButtons:true];
    }

    - (void)clearEntry:(Command)command withResult:(bool)result {
        // 全ての入力欄をクリア
        switch (command) {
            case COMMAND_CCID_PIV_IMPORT_KEY:
                // 全ての入力欄をクリア
                if (result) {
                    [self initTabPkeyCertPathFields];
                    [self initTabPkeyCertPinFields];
                }
                break;
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
                // 全ての入力欄をクリア
                if (result) {
                    [self initTabPinManagementPinFields];
                }
                break;
            default:
                break;
        }
    }

@end
