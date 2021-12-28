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

@end
