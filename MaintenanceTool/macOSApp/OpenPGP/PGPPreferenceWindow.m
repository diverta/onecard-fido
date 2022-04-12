//
//  PGPPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/27.
//
#import "PGPPreferenceWindow.h"
#import "ToolFilePanel.h"
#import "ToolInfoWindow.h"
#import "ToolPGPCommand.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

// 入力可能文字数
#define OPENPGP_NAME_SIZE_MIN               5
#define OPENPGP_ENTRY_SIZE_MAX              32
#define OPENPGP_ADMIN_PIN_CODE_SIZE_MIN     8
#define OPENPGP_ADMIN_PIN_CODE_SIZE_MAX     8
// ASCII項目入力パターン [ -z]（表示可能な半角文字はすべて許容）
#define OPENPGP_ENTRY_PATTERN_ASCII         @"^[ -z]+$"
// ASCII項目入力パターン [ -z]（両端の半角スペースは許容しない）
#define OPENPGP_ENTRY_PATTERN_NOSP_BOTHEND  @"^[!-z]+[ -z]*[!-z]+$"
// メールアドレス入力パターン \w は [a-zA-Z_0-9] と等価
#define OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS  @"^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"

@interface PGPPreferenceWindow () <ToolFilePanelDelegate>

    // ファイル選択用のテキストボックス、ボタン
    @property (assign) IBOutlet NSView              *windowView;
    @property (assign) IBOutlet NSTabView           *tabView;
    @property (assign) IBOutlet NSButton            *buttonClose;
    @property (assign) IBOutlet NSButton            *buttonFirmwareReset;

    @property (assign) IBOutlet NSTabViewItem       *tabPGPKeyManagement;
    @property (assign) IBOutlet NSTextField         *textRealName;
    @property (assign) IBOutlet NSTextField         *textMailAddress;
    @property (assign) IBOutlet NSTextField         *textComment;
    @property (assign) IBOutlet NSTextField         *textPubkeyFolderPath;
    @property (assign) IBOutlet NSTextField         *textBackupFolderPath;
    @property (assign) IBOutlet NSButton            *buttonPubkeyFolderPath;
    @property (assign) IBOutlet NSButton            *buttonBackupFolderPath;
    @property (assign) IBOutlet NSTextField         *textPin;
    @property (assign) IBOutlet NSTextField         *textPinConfirm;
    @property (assign) IBOutlet NSButton            *buttonInstallPGPKey;

    @property (assign) IBOutlet NSTabViewItem       *tabPinManagement;
    @property (assign) IBOutlet NSButton            *buttonChangePin;
    @property (assign) IBOutlet NSButton            *buttonChangeAdminPin;
    @property (assign) IBOutlet NSButton            *buttonUnblockPin;
    @property (assign) IBOutlet NSButton            *buttonSetResetCode;
    @property (assign) IBOutlet NSButton            *buttonUnblock;
    @property (assign) IBOutlet NSTextField         *textCurPin;
    @property (assign) IBOutlet NSTextField         *textNewPin;
    @property (assign) IBOutlet NSTextField         *textNewPinConf;
    @property (assign) IBOutlet NSTextField         *labelCurPin;
    @property (assign) IBOutlet NSTextField         *labelNewPin;
    @property (assign) IBOutlet NSTextField         *labelNewPinConf;
    @property (assign) IBOutlet NSButton            *buttonPerformPinCommand;

    @property (assign) IBOutlet NSButton            *buttonPGPStatus;
    @property (assign) IBOutlet NSButton            *buttonPGPReset;

    // 親画面を保持
    @property (nonatomic, weak) NSWindow            *parentWindow;
    // OpenPGP機能処理クラスの参照を保持
    @property (nonatomic, weak) ToolPGPCommand      *toolPGPCommand;
    @property (nonatomic) ToolFilePanel             *toolFilePanel;
    // 実行するPIN管理コマンドを保持
    @property (nonatomic) Command                    selectedPinCommand;
    @property (nonatomic) NSString                  *selectedPinCommandName;

@end

@implementation PGPPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // PGP鍵管理タブ内の入力項目を初期化（このタブが選択状態になります）
        [[self tabView] selectTabViewItem:[self tabPGPKeyManagement]];
        [self initTabPGPKeyManagement];
        // PIN番号管理タブ内の入力項目を初期化
        [self initTabPinManagement];
    }

    - (void)initTabPGPKeyManagement {
        // テキストボックスの初期化
        [self initTabPGPKeyPathFields];
        [self initTabPGPKeyEntryFields];
    }

    - (void)initTabPGPKeyPathFields {
        // ファイルパスのテキストボックスを初期化
        [[self textPubkeyFolderPath] setStringValue:@""];
        [[self textBackupFolderPath] setStringValue:@""];
        [[self textPubkeyFolderPath] setToolTip:@""];
        [[self textBackupFolderPath] setToolTip:@""];
    }

    - (void)initTabPGPKeyEntryFields {
        // テキストボックスを初期化
        [[self textRealName] setStringValue:@""];
        [[self textMailAddress] setStringValue:@""];
        [[self textComment] setStringValue:@""];
        [[self textPin] setStringValue:@""];
        [[self textPinConfirm] setStringValue:@""];
        // テキストボックスのカーソルを先頭の項目に配置
        [[self textRealName] becomeFirstResponder];
    }

    - (void)initTabPinManagement {
        // ラジオボタンの初期化
        [self initButtonPinCommandsWithDefault:[self buttonChangePin]];
    }

    - (void)initButtonPinCommandsWithDefault:(NSButton *)defaultButton {
        // 「実行する機能」のラジオボタン「PIN番号を変更」を選択状態にする
        [defaultButton setState:NSControlStateValueOn];
        [self getSelectedPinCommandValue:defaultButton];
    }

    - (void)initTabPinManagementPinFields {
        // テキストボックスを初期化
        [[self textCurPin] setStringValue:@""];
        [[self textNewPin] setStringValue:@""];
        [[self textNewPinConf] setStringValue:@""];
        // テキストボックスのカーソルを先頭の項目に配置
        [[self textCurPin] becomeFirstResponder];
    }

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonClose] setEnabled:enabled];
        [[self buttonFirmwareReset] setEnabled:enabled];
        [[self buttonPGPStatus] setEnabled:enabled];
        [[self buttonPGPReset] setEnabled:enabled];
        // 現在選択中のタブ内も同様に制御を行う
        NSTabViewItem *item = [[self tabView] selectedTabViewItem];
        if (item == [self tabPGPKeyManagement]) {
            [self enableButtonsInTabPGPKeyManagement:enabled];
        } else if (item == [self tabPinManagement]) {
            [self enableButtonsInTabPinManagement:enabled];
        }
    }

    - (void)enableButtonsInTabPGPKeyManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self textRealName] setEnabled:enabled];
        [[self textMailAddress] setEnabled:enabled];
        [[self textComment] setEnabled:enabled];
        [[self buttonPubkeyFolderPath] setEnabled:enabled];
        [[self buttonBackupFolderPath] setEnabled:enabled];
        [[self textPin] setEnabled:enabled];
        [[self textPinConfirm] setEnabled:enabled];
        [[self buttonInstallPGPKey] setEnabled:enabled];
    }

    - (void)enableButtonsInTabPinManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonChangePin] setEnabled:enabled];
        [[self buttonChangeAdminPin] setEnabled:enabled];
        [[self buttonUnblockPin] setEnabled:enabled];
        [[self buttonSetResetCode] setEnabled:enabled];
        [[self buttonUnblock] setEnabled:enabled];
        [[self textCurPin] setEnabled:enabled];
        [[self textNewPin] setEnabled:enabled];
        [[self textNewPinConf] setEnabled:enabled];
        [[self buttonPerformPinCommand] setEnabled:enabled];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
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
        [self commandWillPerformPGPProcess:COMMAND_HID_FIRMWARE_RESET withParameter:nil];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合はfalse
        if ([[self toolPGPCommand] checkUSBHIDConnection]) {
            return true;
        }
        // TODO: アラートを表示
        return false;
    }

#pragma mark - For PGPPreferenceWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // ToolPGPCommandの参照を保持
        [self setToolPGPCommand:ref];
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        PGPPreferenceWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithSender:ref modalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        if ([sender isMemberOfClass:[ToolPGPCommand class]]) {
            // ToolPGPCommand経由でメイン画面に制御を戻す
            ToolPGPCommand *command = (ToolPGPCommand *)sender;
            [command commandDidClosePreferenceWindow];
        }
    }

#pragma mark - 鍵・証明書管理タブ関連

    - (IBAction)buttonPubkeyFolderPathDidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER];
    }

    - (IBAction)buttonBackupFolderPathDidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER];
    }

    - (IBAction)buttonInstallPGPKeyDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 入力欄の内容をチェック
        if ([self checkForInstallPGPKey:sender] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_OPENPGP_INSTALL_PGP_KEY informativeText:MSG_PROMPT_INSTALL_PGP_KEY
                                                  withObject:self forSelector:@selector(installPGPKeyCommandPromptDone) parentWindow:[self window]];
    }

    - (void)installPGPKeyCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 画面入力内容をパラメーターに格納
        ToolPGPParameter *parameter = [[ToolPGPParameter alloc] init];
        [parameter setRealName:[[self textRealName] stringValue]];
        [parameter setMailAddress:[[self textMailAddress] stringValue]];
        [parameter setComment:[[self textComment] stringValue]];
        [parameter setPassphrase:[[self textPinConfirm] stringValue]];
        [parameter setPubkeyFolderPath:[[self textPubkeyFolderPath] stringValue]];
        [parameter setBackupFolderPath:[[self textBackupFolderPath] stringValue]];
        // PGP秘密鍵インストール処理を実行
        [self enableButtons:false];
        [self commandWillPerformPGPProcess:COMMAND_OPENPGP_INSTALL_KEYS withParameter:parameter];
    }

    - (IBAction)buttonPGPStatusDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // PGPステータス照会処理を実行
        [self enableButtons:false];
        [self commandWillPerformPGPProcess:COMMAND_OPENPGP_STATUS withParameter:nil];
    }

    - (IBAction)buttonPGPResetDidPress:(id)sender {
        // USBポートに接続されていない場合は終了
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        NSString *msg = [[NSString alloc] initWithFormat:MSG_FORMAT_WILL_PROCESS, MSG_LABEL_COMMAND_OPENPGP_RESET];
        [[ToolPopupWindow defaultWindow] criticalPrompt:msg informativeText:MSG_PROMPT_OPENPGP_RESET
                                             withObject:self forSelector:@selector(PGPResetCommandPromptDone) parentWindow:[self window]];
    }

    - (void)PGPResetCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // PGPリセット処理を実行
        [self enableButtons:false];
        [self commandWillPerformPGPProcess:COMMAND_OPENPGP_RESET withParameter:nil];
    }

    - (void)panelWillSelectPath:(id)sender withPrompt:(NSString *)prompt {
        [[self toolFilePanel] panelWillSelectFolder:sender parentWindow:[self window]
                                         withPrompt:MSG_BUTTON_SELECT withMessage:prompt];
    }

    - (void)panelDidSelectPath:(id)sender filePath:(NSString *)filePath modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse != NSFileHandlingPanelOKButton) {
            return;
        }
        [self setFieldPath:sender filePath:filePath WithField:[self textPubkeyFolderPath] withButton:[self buttonPubkeyFolderPath]];
        [self setFieldPath:sender filePath:filePath WithField:[self textBackupFolderPath] withButton:[self buttonBackupFolderPath]];
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
        // 入力欄の内容をチェック
        if ([self checkForPerformPinCommand:sender] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        NSString *caption = [[NSString alloc] initWithFormat:MSG_FORMAT_OPENPGP_WILL_PROCESS, [self selectedPinCommandName]];
        [[ToolPopupWindow defaultWindow] criticalPrompt:caption informativeText:MSG_PROMPT_OPENPGP_PIN_COMMAND
                                             withObject:self forSelector:@selector(performPinCommandPromptDone) parentWindow:[self window]];
    }

    - (void)performPinCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // 画面入力内容をパラメーターに格納
        ToolPGPParameter *parameter = [[ToolPGPParameter alloc] init];
        [parameter setPinCommandName:[self selectedPinCommandName]];
        [parameter setCurrentPin:[[self textCurPin] stringValue]];
        [parameter setRenewalPin:[[self textNewPin] stringValue]];
        // PIN番号管理コマンドを実行
        [self enableButtons:false];
        [self commandWillPerformPGPProcess:[self selectedPinCommand] withParameter:parameter];
    }

    - (void)getSelectedPinCommandValue:(NSButton *)button {
        // ラジオボタンの選択状態に応じ、入力欄のキャプションも変更する
        if (button == [self buttonChangePin]) {
            // PIN番号を変更
            [self setSelectedPinCommand:COMMAND_OPENPGP_CHANGE_PIN];
            [self setSelectedPinCommandName:MSG_LABEL_COMMAND_OPENPGP_CHANGE_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_ITEM_CUR_PIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_ITEM_NEW_PIN];
        }
        if (button == [self buttonChangeAdminPin]) {
            // 管理用PIN番号を変更
            [self setSelectedPinCommand:COMMAND_OPENPGP_CHANGE_ADMIN_PIN];
            [self setSelectedPinCommandName:MSG_LABEL_COMMAND_OPENPGP_CHANGE_ADMIN_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_ITEM_CUR_ADMPIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_ITEM_NEW_ADMPIN];
        }
        if (button == [self buttonUnblockPin]) {
            // PIN番号をリセット
            [self setSelectedPinCommand:COMMAND_OPENPGP_UNBLOCK_PIN];
            [self setSelectedPinCommandName:MSG_LABEL_COMMAND_OPENPGP_UNBLOCK_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_ITEM_CUR_ADMPIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_ITEM_NEW_PIN];
        }
        if (button == [self buttonSetResetCode]) {
            // リセットコードを変更
            [self setSelectedPinCommand:COMMAND_OPENPGP_SET_RESET_CODE];
            [self setSelectedPinCommandName:MSG_LABEL_COMMAND_OPENPGP_SET_RESET_CODE];
            [[self labelCurPin] setStringValue:MSG_LABEL_ITEM_CUR_ADMPIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_ITEM_NEW_RESET_CODE];
        }
        if (button == [self buttonUnblock]) {
            // リセットコードでPIN番号をリセット
            [self setSelectedPinCommand:COMMAND_OPENPGP_UNBLOCK];
            [self setSelectedPinCommandName:MSG_LABEL_COMMAND_OPENPGP_UNBLOCK];
            [[self labelCurPin] setStringValue:MSG_LABEL_ITEM_CUR_RESET_CODE];
            [[self labelNewPin] setStringValue:MSG_LABEL_ITEM_NEW_PIN];
        }
        // 確認欄のキャプションを設定
        NSString *caption = [[NSString alloc] initWithFormat:MSG_FORMAT_OPENPGP_ITEM_FOR_CONF, [[self labelNewPin] stringValue]];
        [[self labelNewPinConf] setStringValue:caption];
        // PIN入力欄をクリアし、新しいPIN欄にフォーカスを移す
        [self initTabPinManagementPinFields];
    }

#pragma mark - 入力チェック関連

    - (bool)checkForInstallPGPKey:(id)sender {
        // 入力欄のチェック
        if ([self checkMustEntry:[self textRealName] fieldName:MSG_LABEL_PGP_REAL_NAME sizeMin:OPENPGP_NAME_SIZE_MIN sizeMax:OPENPGP_ENTRY_SIZE_MAX] == false) {
            return false;
        }
        if ([self checkAsciiEntry:[self textRealName] fieldName:MSG_LABEL_PGP_REAL_NAME] == false) {
            return false;
        }
        if ([self checkEntryNoSpaceExistOnBothEnds:[self textRealName] fieldName:MSG_LABEL_PGP_REAL_NAME] == false) {
            return false;
        }
        if ([self checkMustEntry:[self textMailAddress] fieldName:MSG_LABEL_PGP_MAIL_ADDRESS sizeMin:1 sizeMax:OPENPGP_ENTRY_SIZE_MAX] == false) {
            return false;
        }
        if ([self checkAddressEntry:[self textMailAddress] fieldName:MSG_LABEL_PGP_MAIL_ADDRESS] == false) {
            return false;
        }
        if ([self checkMustEntry:[self textComment] fieldName:MSG_LABEL_PGP_COMMENT sizeMin:1 sizeMax:OPENPGP_ENTRY_SIZE_MAX] == false) {
            return false;
        }
        if ([self checkAsciiEntry:[self textComment] fieldName:MSG_LABEL_PGP_COMMENT] == false) {
            return false;
        }
        if ([self checkEntryNoSpaceExistOnBothEnds:[self textComment] fieldName:MSG_LABEL_PGP_COMMENT] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textPubkeyFolderPath] messageIfError:MSG_PROMPT_SELECT_PGP_PUBKEY_FOLDER] == false) {
            return false;
        }
        if ([self checkPathEntry:[self textBackupFolderPath] messageIfError:MSG_PROMPT_SELECT_PGP_BACKUP_FOLDER] == false) {
            return false;
        }
        if ([self checkPinNumber:[self textPin] withName:MSG_LABEL_PGP_ADMIN_PIN] == false) {
            return false;
        }
        if ([self checkPinNumber:[self textPinConfirm] withName:MSG_LABEL_PGP_ADMIN_PIN_CONFIRM] == false) {
            return false;
        }
        // 確認用PINコードのチェック
        if ([self checkPinConfirmFor:[self textPinConfirm] withSource:[self textPin]
                            withName:MSG_LABEL_PGP_ADMIN_PIN_CONFIRM] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkMustEntry:(NSTextField *)field fieldName:(NSString *)fieldName sizeMin:(int)min sizeMax:(int)max {
        // 必須チェック
        NSString *message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_MUST_ENTRY, fieldName];
        if ([ToolCommon checkMustEntry:field informativeText:message] == false) {
            return false;
        }
        // 長さチェック
        message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT, fieldName, min, max];
        if ([ToolCommon checkEntrySize:field minSize:min maxSize:max informativeText:message] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkAsciiEntry:(NSTextField *)field fieldName:(NSString *)fieldName {
        // 入力パターンチェック
        NSString *message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ASCII_ENTRY, fieldName];
        if ([ToolCommon checkValueWithPattern:field pattern:OPENPGP_ENTRY_PATTERN_ASCII informativeText:message] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkAddressEntry:(NSTextField *)field fieldName:(NSString *)fieldName {
        // 入力パターンチェック
        NSString *message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ADDRESS_ENTRY, fieldName];
        if ([ToolCommon checkValueWithPattern:field pattern:OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS informativeText:message] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkEntryNoSpaceExistOnBothEnds:(NSTextField *)field fieldName:(NSString *)fieldName {
        // 入力パターンチェック
        NSString *message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ENTRY_NOSP_BOTHEND, fieldName];
        if ([ToolCommon checkValueWithPattern:field pattern:OPENPGP_ENTRY_PATTERN_NOSP_BOTHEND informativeText:message] == false) {
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
        NSString *msg1 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ADMIN_PIN_DIGIT, name];
        if ([ToolCommon checkEntrySize:field minSize:OPENPGP_ADMIN_PIN_CODE_SIZE_MIN maxSize:OPENPGP_ADMIN_PIN_CODE_SIZE_MAX
                       informativeText:msg1] == false) {
            return false;
        }
        // 数字チェック
        NSString *msg2 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, name];
        if ([ToolCommon checkIsNumeric:field informativeText:msg2] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPinConfirmFor:(NSTextField *)dest withSource:(NSTextField *)source withName:(NSString *)name {
        // PIN番号の確認入力内容をチェック
        NSString *msg = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ADMIN_PIN_CONFIRM, name];
        return [ToolCommon compareEntry:dest srcField:source informativeText:msg];
    }

    - (bool)checkForPerformPinCommand:(id)sender {
        // チェック用パラメーターの設定
        NSString *msgCurPin = @"";
        NSString *msgNewPin = @"";
        int minSizeCurPin = 0;
        int minSizeNewPin = 0;
        switch ([self selectedPinCommand]) {
            case COMMAND_OPENPGP_CHANGE_PIN:
                 msgCurPin = MSG_LABEL_ITEM_CUR_PIN;
                 minSizeCurPin = 6;
                 msgNewPin = MSG_LABEL_ITEM_NEW_PIN;
                 minSizeNewPin = 6;
                 break;
            case COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
                 msgCurPin = MSG_LABEL_ITEM_CUR_ADMPIN;
                 minSizeCurPin = 8;
                 msgNewPin = MSG_LABEL_ITEM_NEW_ADMPIN;
                 minSizeNewPin = 8;
                 break;
            case COMMAND_OPENPGP_UNBLOCK_PIN:
                msgCurPin = MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            case COMMAND_OPENPGP_SET_RESET_CODE:
                msgCurPin = MSG_LABEL_ITEM_CUR_ADMPIN;
                minSizeCurPin = 8;
                msgNewPin = MSG_LABEL_ITEM_NEW_RESET_CODE;
                minSizeNewPin = 8;
                break;
            case COMMAND_OPENPGP_UNBLOCK:
                msgCurPin = MSG_LABEL_ITEM_CUR_RESET_CODE;
                minSizeCurPin = 8;
                msgNewPin = MSG_LABEL_ITEM_NEW_PIN;
                minSizeNewPin = 6;
                break;
            default:
                break;
        }
        // 現在のPINをチェック
        if ([self checkPinNumbersForPinCommand:[self textCurPin] fieldName:msgCurPin sizeMin:minSizeCurPin] == false) {
            return false;
        }
        // 新しいPINをチェック
        if ([self checkPinNumbersForPinCommand:[self textNewPin] fieldName:msgNewPin sizeMin:minSizeNewPin] == false) {
            return false;
        }
        // 確認用PINのラベル
        NSString *msgNewPinConf = [[NSString alloc] initWithFormat:MSG_FORMAT_OPENPGP_ITEM_FOR_CONFIRM, msgNewPin];
        // 確認用PINコードのチェック
        if ([self checkPinConfirmFor:[self textNewPinConf] withSource:[self textNewPin] withName:msgNewPinConf] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkPinNumbersForPinCommand:(NSTextField *)field fieldName:(NSString *)name sizeMin:(int)min {
        // 長さチェック
        NSString *msg1 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_PIN_DIGIT, name, min];
        if ([ToolCommon checkEntrySize:field minSize:min maxSize:min informativeText:msg1] == false) {
            return false;
        }
        // 数字チェック
        NSString *msg2 = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ADMIN_PIN_NUM, name];
        if ([ToolCommon checkIsNumeric:field informativeText:msg2] == false) {
            return false;
        }
        return true;
    }

#pragma mark - For ToolPGPCommand functions

    - (void)commandWillPerformPGPProcess:(Command)command withParameter:(ToolPGPParameter *)parameter {
        // 進捗画面を表示
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self window]];
        // OpenPGP機能を実行
        [[self toolPGPCommand] commandWillPerformPGPProcess:command withParameter:parameter];
    }

    - (void)toolPGPCommandDidProcess:(Command)command withResult:(bool)result withErrorMessage:(NSString *)errorMessage {
        // 進捗画面を閉じる
        [[ToolProcessingWindow defaultWindow] windowWillClose:NSModalResponseOK];
        // 処理終了メッセージをポップアップ表示後、画面項目を使用可とする
        [self displayResultMessage:command withResult:result withErrorMessage:errorMessage];
        [self clearEntry:command withResult:result];
        [self enableButtons:true];
    }

    - (void)displayResultMessage:(Command)command withResult:(bool)result withErrorMessage:(NSString *)errorMessage {
        // 処理名称を設定
        NSString *name = nil;
        switch (command) {
            case COMMAND_OPENPGP_INSTALL_KEYS:
                name = MSG_LABEL_COMMAND_OPENPGP_INSTALL_KEYS;
                break;
            case COMMAND_OPENPGP_RESET:
                name = MSG_LABEL_COMMAND_OPENPGP_RESET;
                break;
            case COMMAND_OPENPGP_STATUS:
                if (result) {
                    // メッセージの代わりに、OpenPGP設定情報を、情報表示画面に表示
                    [[ToolInfoWindow defaultWindow] windowWillOpenWithCommandRef:[self toolPGPCommand]
                        withParentWindow:[self window] titleString:PROCESS_NAME_OPENPGP_STATUS
                        infoString:[[self toolPGPCommand] getPGPStatusInfoString]];
                    return;
                }
                name = MSG_LABEL_COMMAND_OPENPGP_STATUS;
                break;
            case COMMAND_HID_FIRMWARE_RESET:
                name = PROCESS_NAME_FIRMWARE_RESET;
                break;
            case COMMAND_OPENPGP_CHANGE_PIN:
            case COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
            case COMMAND_OPENPGP_UNBLOCK_PIN:
            case COMMAND_OPENPGP_SET_RESET_CODE:
            case COMMAND_OPENPGP_UNBLOCK:
                name = [self selectedPinCommandName];
                break;
            default:
                return;
        }
        // メッセージをポップアップ表示
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, name,
                         result ? MSG_SUCCESS:MSG_FAILURE];
        if (result) {
            [ToolPopupWindow informational:str informativeText:nil];
        } else {
            [ToolPopupWindow critical:str informativeText:errorMessage];
        }
    }

    - (void)clearEntry:(Command)command withResult:(bool)result {
        // 全ての入力欄をクリア
        switch (command) {
            case COMMAND_OPENPGP_INSTALL_KEYS:
                if (result) {
                    [self initTabPGPKeyPathFields];
                    [self initTabPGPKeyEntryFields];
                }
                break;
            case COMMAND_OPENPGP_CHANGE_PIN:
            case COMMAND_OPENPGP_CHANGE_ADMIN_PIN:
            case COMMAND_OPENPGP_UNBLOCK_PIN:
            case COMMAND_OPENPGP_SET_RESET_CODE:
            case COMMAND_OPENPGP_UNBLOCK:
                if (result) {
                    [self initTabPinManagementPinFields];
                }
                break;
            default:
                break;
        }
    }

@end
