//
//  FIDOAttestationWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/12.
//
#import "AppCommonMessage.h"
#import "FIDOAttestationWindow.h"
#import "ToolCommonFunc.h"
#import "ToolFilePanel.h"
#import "VendorFunctionCommand.h"

@interface FIDOAttestationWindow () <ToolFilePanelDelegate>

    // 画面項目
    @property (assign) IBOutlet NSTextField                *textPkeyPemPath;
    @property (assign) IBOutlet NSTextField                *textCertPemPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectPkeyPemPath;
    @property (assign) IBOutlet NSButton                   *buttonSelectCertPemPath;
    // 親画面の参照を保持
    @property (nonatomic) NSWindow                         *parentWindow;
    // コマンドクラスの参照を保持
    @property (nonatomic, weak) VendorFunctionCommand      *commandRef;
    @property (nonatomic) ToolFilePanel                    *toolFilePanel;
    // 処理パラメーターを保持
    @property (nonatomic) VendorFunctionCommandParameter   *commandParameterRef;

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
        [[self textPkeyPemPath] setStringValue:@""];
        [[self textPkeyPemPath] setToolTip:@""];
        [[self textCertPemPath] setStringValue:@""];
        [[self textCertPemPath] setToolTip:@""];
    }

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
        // コマンドクラスの参照を保持
        [self setCommandRef:commandRef];
        // ヘルスチェック処理のパラメーターを保持
        [self setCommandParameterRef:parameterRef];
        // パラメーターを初期化
        [[self commandParameterRef] setCommand:COMMAND_NONE];
    }

    - (IBAction)buttonSelectPkeyPemPathDidPress:(id)sender {
        // 鍵ファイルのパス選択画面を表示
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_PKEY_PATH withFileTypes:@[@"pem"]];
    }

    - (IBAction)buttonSelectCertPemPathDidPress:(id)sender {
        // 証明書ファイルのパス選択画面を表示
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:MSG_PROMPT_SELECT_CRT_PATH withFileTypes:@[@"crt"]];
    }

    - (IBAction)buttonInstallDidPress:(id)sender {
        // USBポートに接続されていない場合は処理中止
        if ([self checkUSBHIDConnection] == false) {
            return;
        }
        // 選択された鍵・証明書ファイルのパスをチェック
        if ([self checkPathEntry:[self textPkeyPemPath] messageIfError:MSG_PROMPT_SELECT_PKEY_PATH] == false) {
            return;
        }
        if ([self checkPathEntry:[self textCertPemPath] messageIfError:MSG_PROMPT_SELECT_CRT_PATH] == false) {
            return;
        }
        // 選択された鍵・証明書ファイルのパスを保持
        [[self commandParameterRef] setPkeyPemPath:[[self textPkeyPemPath] toolTip]];
        [[self commandParameterRef] setCertPemPath:[[self textCertPemPath] toolTip]];
        // 画面項目を初期化
        [self initFieldValue];
        // この画面を閉じる
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // この画面を閉じる
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

    - (bool)checkUSBHIDConnection {
        // USBポートに接続されていない場合は処理中止
        return [ToolCommonFunc checkUSBHIDConnectionOnWindow:[self window] connected:[[self commandRef] isUSBHIDConnected]];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse == NSFileHandlingPanelOKButton) {
            // ファイルパスをディレクトリー、ファイル名に分割し、テキストボックスにはファイル名のみ設定
            NSString *fileName = [filePath lastPathComponent];
            if (sender == [self buttonSelectPkeyPemPath]) {
                [[self textPkeyPemPath] setStringValue:fileName];
                [[self textPkeyPemPath] setToolTip:filePath];
            }
            if (sender == [self buttonSelectCertPemPath]) {
                [[self textCertPemPath] setStringValue:fileName];
                [[self textCertPemPath] setToolTip:filePath];
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
        if ([ToolCommonFunc checkFileExist:field forPath:[field toolTip] informativeText:message onWindow:[self window]] == false) {
            return false;
        }
        return true;
    }

@end
