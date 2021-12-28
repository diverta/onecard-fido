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
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

// 入力可能文字数
#define OPENPGP_ENTRY_SIZE_MAX              32
#define OPENPGP_ADMIN_PIN_CODE_SIZE_MIN     8
#define OPENPGP_ADMIN_PIN_CODE_SIZE_MAX     8
// ASCII項目入力パターン [ -z]（表示可能な半角文字はすべて許容）
#define OPENPGP_ENTRY_PATTERN_ASCII         @"([ -z]+)"
// メールアドレス入力パターン \w は [a-zA-Z_0-9] と等価
#define OPENPGP_ENTRY_PATTERN_MAIL_ADDRESS  @"^\\w+([-+.]\\w+)*@\\w+([-.]\\w+)*\\.\\w+([-.]\\w+)*$"

@interface PGPPreferenceWindow () <ToolFilePanelDelegate>

    // ファイル選択用のテキストボックス、ボタン
    @property (assign) IBOutlet NSView              *windowView;
    @property (assign) IBOutlet NSTabView           *tabView;
    @property (assign) IBOutlet NSButton            *buttonClose;

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

    // 親画面を保持
    @property (nonatomic, weak) NSWindow            *parentWindow;
    // OpenPGP機能処理クラスの参照を保持
    @property (nonatomic, weak) ToolPGPCommand      *toolPGPCommand;
    @property (nonatomic) ToolFilePanel             *toolFilePanel;

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

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonClose] setEnabled:enabled];
        // 現在選択中のタブ内も同様に制御を行う
        NSTabViewItem *item = [[self tabView] selectedTabViewItem];
        if (item == [self tabPGPKeyManagement]) {
            [self enableButtonsInTabPGPKeyManagement:enabled];
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

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
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
        // 入力欄の内容をチェック
        if ([self checkForInstallPGPKey:sender]) {
            // TODO: 仮の実装です。
            [ToolPopupWindow informational:MSG_CMDTST_MENU_NOT_SUPPORTED informativeText:nil];
        }
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

#pragma mark - 入力チェック関連

    - (bool)checkForInstallPGPKey:(id)sender {
        // 入力欄のチェック
        if ([self checkMustEntry:[self textRealName] fieldName:MSG_LABEL_PGP_REAL_NAME] == false) {
            return false;
        }
        if ([self checkAsciiEntry:[self textRealName] fieldName:MSG_LABEL_PGP_REAL_NAME] == false) {
            return false;
        }
        if ([self checkMustEntry:[self textMailAddress] fieldName:MSG_LABEL_PGP_MAIL_ADDRESS] == false) {
            return false;
        }
        if ([self checkAddressEntry:[self textMailAddress] fieldName:MSG_LABEL_PGP_MAIL_ADDRESS] == false) {
            return false;
        }
        if ([self checkMustEntry:[self textComment] fieldName:MSG_LABEL_PGP_COMMENT] == false) {
            return false;
        }
        if ([self checkAsciiEntry:[self textComment] fieldName:MSG_LABEL_PGP_COMMENT] == false) {
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
        // 事前に確認ダイアログを表示
        if ([ToolPopupWindow promptYesNo:MSG_OPENPGP_INSTALL_PGP_KEY
                         informativeText:MSG_PROMPT_INSTALL_PGP_KEY] == false) {
            return false;
        }
        return true;
    }

    - (bool)checkMustEntry:(NSTextField *)field fieldName:(NSString *)fieldName {
        // 必須チェック
        NSString *message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_MUST_ENTRY, fieldName];
        if ([ToolCommon checkMustEntry:field informativeText:message] == false) {
            return false;
        }
        // 長さチェック
        message = [[NSString alloc] initWithFormat:MSG_PROMPT_INPUT_PGP_ENTRY_DIGIT, fieldName];
        if ([ToolCommon checkEntrySize:field minSize:1 maxSize:OPENPGP_ENTRY_SIZE_MAX informativeText:message] == false) {
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

@end
