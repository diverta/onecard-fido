//
//  FIDOAttestationWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/11/17.
//
#import "FIDOAttestationWindow.h"
#import "ToolCommonMessage.h"
#import "ToolFIDOAttestationCommand.h"
#import "ToolPopupWindow.h"
#import "ToolFilePanel.h"

@interface FIDOAttestationWindow () <ToolFilePanelDelegate>

    // 画面項目
    @property (assign) IBOutlet NSTextField                *textKeyPath;
    @property (assign) IBOutlet NSTextField                *textCertPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectKeyPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectCertPath;
    @property (assign) IBOutlet NSButton                   *buttonInstall;
    @property (assign) IBOutlet NSButton                   *buttonDelete;
    @property (assign) IBOutlet NSButton                   *buttonCancel;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) ToolFIDOAttestationCommand *toolFIDOAttestationCommand;
    @property (nonatomic) ToolFilePanel                    *toolFilePanel;
    // 画面で選択された鍵・証明書ファイルパスを保持
    @property (nonatomic) NSArray<NSString *>              *filePaths;
    // 実行するコマンドを保持
    @property (nonatomic) Command                           command;

@end

@implementation FIDOAttestationWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        // コマンドクラスの初期化
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        [[self textKeyPath] setStringValue:@""];
        [[self textCertPath] setStringValue:@""];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setCommandRef:(id)ref {
        // コマンドクラスの参照を保持
        [self setToolFIDOAttestationCommand:(ToolFIDOAttestationCommand *)ref];
    }

    - (IBAction)buttonSelectKeyPathDidPress:(id)sender {
        // 鍵ファイルのパス選択画面を表示
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_PKEY_PATH withFileTypes:@[@"pem"]];
    }

    - (IBAction)buttonSelectCertPathDidPress:(id)sender {
        // 証明書ファイルのパス選択画面を表示
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_CRT_PATH withFileTypes:@[@"crt"]];
    }

    - (IBAction)buttonInstallDidPress:(id)sender {
        // 選択された鍵・証明書ファイルのパスをチェック
        if ([self checkPathEntry:[self textKeyPath] messageIfError:MSG_PROMPT_SELECT_PKEY_PATH] == false) {
            return;
        }
        if ([self checkPathEntry:[self textCertPath] messageIfError:MSG_PROMPT_SELECT_CRT_PATH] == false) {
            return;
        }
        // 選択された鍵・証明書ファイルのパスを保持
        NSArray<NSString *> *paths = @[[[self textKeyPath] stringValue], [[self textCertPath] stringValue]];
        [self setFilePaths:paths];
        // 処理開始前に確認
        if ([[self toolFIDOAttestationCommand] checkUSBHIDConnection]) {
            if ([ToolPopupWindow promptYesNo:MSG_INSTALL_SKEY_CERT
                             informativeText:MSG_PROMPT_INSTL_SKEY_CERT]) {
                // この画面を閉じ、鍵・証明書インストール処理を開始
                [self terminateWindow:NSModalResponseOK withCommand:COMMAND_INSTALL_SKEY_CERT];
            }
        }
    }

    - (IBAction)buttonDeleteDidPress:(id)sender {
        // 処理開始前に確認
        if ([[self toolFIDOAttestationCommand] checkUSBHIDConnection]) {
            if ([ToolPopupWindow promptYesNo:MSG_ERASE_SKEY_CERT
                             informativeText:MSG_PROMPT_ERASE_SKEY_CERT]) {
                // この画面を閉じ、鍵・証明書削除処理を開始
                [self terminateWindow:NSModalResponseOK withCommand:COMMAND_ERASE_SKEY_CERT];
            }
        }
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // この画面を閉じる
        [self terminateWindow:NSModalResponseCancel withCommand:COMMAND_NONE];
    }

    - (void)terminateWindow:(NSModalResponse)response withCommand:(Command)command {
        // 実行コマンドを保持
        [self setCommand:command];
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Interface for parameters

    - (Command)commandToPerform {
        // 実行コマンドを戻す
        return [self command];
    }

    - (NSArray<NSString *> *)selectedFilePaths {
        // 選択された鍵・証明書ファイルのパスを戻す
        return [self filePaths];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse == NSFileHandlingPanelOKButton) {
            if ([self buttonSelectKeyPath] == sender) {
                [[self textKeyPath] setStringValue:filePath];
                [[self textKeyPath] setToolTip:filePath];
            }
            if ([self buttonSelectCertPath] == sender) {
                [[self textCertPath] setStringValue:filePath];
                [[self textCertPath] setToolTip:filePath];
            }
        }
    }

#pragma mark - Check for entries and process

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

@end
