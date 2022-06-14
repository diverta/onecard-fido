//
//  FIDOAttestationWindow.m
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/14.
//
#import "AppCommonMessage.h"
#import "AppDefine.h"
#import "FIDOAttestationWindow.h"
#import "FIDOSettingCommand.h"
#import "ToolCommonFunc.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"

@interface FIDOAttestationWindow () <ToolFilePanelDelegate>

    // 画面項目
    @property (assign) IBOutlet NSTextField                *textKeyPath;
    @property (assign) IBOutlet NSTextField                *textCertPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectKeyPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectCertPath;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) FIDOSettingCommand         *fidoSettingCommand;
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
        [self setFidoSettingCommand:(FIDOSettingCommand *)ref];
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
        // USBポートに接続されていない場合は処理中止
        if ([[self fidoSettingCommand] checkUSBHIDConnection] == false) {
            [[ToolPopupWindow defaultWindow] critical:MSG_PROMPT_USB_PORT_SET informativeText:nil
                                           withObject:nil forSelector:nil parentWindow:[self window]];
            return;
        }
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
        [[ToolPopupWindow defaultWindow] informationalPrompt:MSG_INSTALL_SKEY_CERT informativeText:MSG_PROMPT_INSTL_SKEY_CERT
                                                  withObject:self forSelector:@selector(installCommandPromptDone) parentWindow:[self window]];
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

    - (void)installCommandPromptDone {
        // ポップアップでデフォルトのNoボタンがクリックされた場合は、以降の処理を行わない
        if ([[ToolPopupWindow defaultWindow] isButtonNoClicked]) {
            return;
        }
        // この画面を閉じ、鍵・証明書インストール処理を開始
        [self terminateWindow:NSModalResponseOK withCommand:COMMAND_FIDO_ATTESTATION_INSTALL];
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
        if ([ToolCommonFunc checkMustEntry:field informativeText:message onWindow:[self window]] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolCommonFunc checkFileExist:field informativeText:message onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

@end
