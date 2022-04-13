//
//  PIVPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#import "PIVPreferenceWindow.h"
#import "ToolFilePanel.h"
#import "ToolInfoWindow.h"
#import "ToolPIVCommand.h"
#import "ToolPIVImporter.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"
#import "ToolCommonMessage.h"
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
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId1;
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId2;
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId3;
    @property (assign) IBOutlet NSTextField         *fieldPath1;
    @property (assign) IBOutlet NSTextField         *fieldPath2;
    @property (assign) IBOutlet NSButton            *buttonPath1;
    @property (assign) IBOutlet NSButton            *buttonPath2;
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
    @property (nonatomic) uint8_t                    selectedPkeySlotId;
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
        // ラジオボタンの初期化
        [self initButtonPkeySlotIdsWithDefault:[self buttonPkeySlotId1]];
        // テキストボックスの初期化
        [self initTabPkeyCertPathFields];
        [self initTabPkeyCertPinFields];
    }

    - (void)initTabPkeyCertPathFields {
        // ファイルパスのテキストボックスを初期化
        [[self fieldPath1] setStringValue:@""];
        [[self fieldPath2] setStringValue:@""];
        [[self fieldPath1] setToolTip:@""];
        [[self fieldPath2] setToolTip:@""];
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
        [[self buttonPkeySlotId1] setEnabled:enabled];
        [[self buttonPkeySlotId2] setEnabled:enabled];
        [[self buttonPkeySlotId3] setEnabled:enabled];
        [[self fieldPath1] setEnabled:enabled];
        [[self fieldPath2] setEnabled:enabled];
        [[self buttonPath1] setEnabled:enabled];
        [[self buttonPath2] setEnabled:enabled];
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
        [self commandWillStatus];
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
        [self commandWillSetCHUIDAndCCC];
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
        [self commandWillReset];
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
        [self commandWillResetFirmware];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if ([[self toolPIVCommand] checkUSBHIDConnection]) {
            return true;
        }
        // エラーメッセージをポップアップ表示
        [[ToolPopupWindow defaultWindow] critical:MSG_CMDTST_PROMPT_USB_PORT_SET informativeText:nil withObject:nil forSelector:nil];
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

    - (IBAction)buttonPkeySlotIdSelected:(id)sender {
        [self getSelectedPkeySlotIdValue:sender];
    }

    - (IBAction)buttonPath1DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }

    - (IBAction)buttonPath2DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH];
    }

    - (IBAction)buttonInstallPkeyCertDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // ラジオボタンから鍵種別を取得
        uint8_t slotId = [self selectedPkeySlotId];
        // 入力欄の内容をチェック
        if ([self checkForInstallPkeyCert:sender toKeySlot:slotId] == false) {
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
        // ラジオボタンから鍵種別を取得
        uint8_t slotId = [self selectedPkeySlotId];
        // PIV認証用の鍵・証明書インストール（ラジオボタンから鍵種別を取得）
        NSString *pkeyPemPath = [[self fieldPath1] stringValue];
        NSString *certPemPath = [[self fieldPath2] stringValue];
        NSString *authPin = [[self fieldPin2] stringValue];
        [self commandWillImportPkeyCert:slotId pkeyPemPath:pkeyPemPath certPemPath:certPemPath withAuthPin:authPin];
    }

    - (void)initButtonPkeySlotIdsWithDefault:(NSButton *)defaultButton {
        // 「インストールする鍵・証明書」のラジオボタン「PIV認証用」を選択状態にする
        [defaultButton setState:NSControlStateValueOn];
        [self getSelectedPkeySlotIdValue:defaultButton];
    }

    - (void)getSelectedPkeySlotIdValue:(NSButton *)button {
        if (button == [self buttonPkeySlotId1]) {
            [self setSelectedPkeySlotId:0x9a];
        } else if (button == [self buttonPkeySlotId2]) {
            [self setSelectedPkeySlotId:0x9c];
        } else if (button == [self buttonPkeySlotId3]) {
            [self setSelectedPkeySlotId:0x9d];
        } else {
            [self setSelectedPkeySlotId:0x00];
        }
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
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath1] withButton:[self buttonPath1]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath2] withButton:[self buttonPath2]];
    }

    - (void)setFieldPath:(id)sender filePath:(NSString *)filePath WithField:(NSTextField *)field withButton:(NSButton *)button {
        if (sender == button) {
            [field setStringValue:filePath];
            [field setToolTip:filePath];
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
        // PIN番号管理コマンドを実行（ラジオボタンから実行コマンドを取得）
        NSString *curPin = [[self fieldCurPin] stringValue];
        NSString *newPin = [[self fieldNewPinConf] stringValue];
        [self commandWillChangePin:command withNewPin:newPin withAuthPin:curPin];
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

    - (bool)checkForInstallPkeyCert:(id)sender toKeySlot:(uint8_t)slotId {
        // 入力欄のチェック
        if ([self checkPathEntry:[self fieldPath1] messageIfError:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH] == false) {
            return false;
        }
        if ([self checkPathEntry:[self fieldPath2] messageIfError:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH] == false) {
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
        if ([ToolCommon checkMustEntry:field informativeText:message] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolCommon checkFileExist:field informativeText:message] == false) {
            return false;
        }
        return true;
    }

    - (bool) checkPinNumber:(NSTextField *)field withName:(NSString *)name {
        // 長さチェック
        NSString *msg1 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_DIGIT, name];
        if ([ToolCommon checkEntrySize:field minSize:PIV_PIN_CODE_SIZE_MIN maxSize:PIV_PIN_CODE_SIZE_MAX
                       informativeText:msg1] == false) {
            return false;
        }
        // 数字チェック
        NSString *msg2 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_NUM, name];
        if ([ToolCommon checkIsNumeric:field informativeText:msg2] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPinConfirmFor:(NSTextField *)dest withSource:(NSTextField *)source withName:(NSString *)name {
        NSString *msg = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PIV_PIN_PUK_CONFIRM, name];
        return [ToolCommon compareEntry:dest srcField:source informativeText:msg];
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

    - (void)commandWillResetFirmware {
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillResetFirmware:COMMAND_HID_FIRMWARE_RESET];
    }

    - (void)commandWillStatus {
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillStatus:COMMAND_CCID_PIV_STATUS];
    }

    - (void)commandWillSetCHUIDAndCCC {
        ToolPIVImporter *importer = [[ToolPIVImporter alloc] init];
        [importer generateChuidAndCcc];
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillSetCHUIDAndCCC:COMMAND_CCID_PIV_SET_CHUID withImporterRef:importer];
    }

    - (void)commandWillReset {
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillReset:COMMAND_CCID_PIV_RESET];
    }

    - (void)commandWillImportPkeyCert:(uint8_t)keySlotId pkeyPemPath:(NSString *)pkey certPemPath:(NSString *)cert withAuthPin:(NSString *)authPin {
        ToolPIVImporter *importer = [[ToolPIVImporter alloc] initForKeySlot:keySlotId];
        if ([importer readPrivateKeyPemFrom:pkey] == false) {
            [[ToolPopupWindow defaultWindow] critical:MSG_PIV_LOAD_PKEY_FAILED informativeText:nil withObject:nil forSelector:nil];
            return;
        }
        if ([importer readCertificatePemFrom:cert] == false) {
            [[ToolPopupWindow defaultWindow] critical:MSG_PIV_LOAD_CERT_FAILED informativeText:nil withObject:nil forSelector:nil];
            return;
        }
        // 鍵・証明書のアルゴリズムが異なる場合は、エラーメッセージを表示し処理中止
        if ([importer keyAlgorithm] != [importer certAlgorithm]) {
            NSString *info = [[NSString alloc] initWithFormat:MSG_FORMAT_PIV_PKEY_CERT_ALGORITHM,
                              [importer keyAlgorithm], [importer certAlgorithm]];
            [[ToolPopupWindow defaultWindow] critical:MSG_PIV_PKEY_CERT_ALGORITHM_CMP_FAILED informativeText:info withObject:nil forSelector:nil];
            return;
        }
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillImportKey:COMMAND_CCID_PIV_IMPORT_KEY withAuthPinCode:authPin withImporterRef:importer];
    }

    - (void)commandWillChangePin:(Command)command withNewPin:(NSString *)newPin withAuthPin:(NSString *)authPin {
        // 進捗画面を表示し、コマンドを実行
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        [[self toolPIVCommand] commandWillChangePin:command withNewPinCode:newPin withAuthPinCode:authPin];
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
