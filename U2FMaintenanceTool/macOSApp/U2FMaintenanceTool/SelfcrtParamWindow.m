//
//  SelfcrtParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#import "SelfcrtParamWindow.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

@interface SelfcrtParamWindow () <ToolFilePanelDelegate>

    @property (assign) IBOutlet NSTextField *fieldPath;

    @property (nonatomic) ToolFilePanel *toolFilePanel;

@end

@implementation SelfcrtParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        [self setParameter:[[SelfCertParameter alloc] init]];
    }

    - (IBAction)buttonFieldPathPress:(id)sender {
        // ファイル選択パネルをモーダル表示
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT
                                       message:MSG_PROMPT_SELECT_CSR_PATH
                                     fileTypes:@[@"csr"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        [self doProcess:sender];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // 画面入力値をパラメーターに保持
        [[self parameter] setCsrPath:[[self fieldPath] stringValue]];
        
        // パラメーターに保持した入力項目を初期化
        [[self fieldPath] setStringValue:@""];
        
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Check for entries and process

    - (void) doProcess:(id)sender {
        // 入力内容チェックがOKであれば、ファイル保存パネルをモーダル表示
        if ([self checkEntries] == false) {
            return;
        }
        [[self toolFilePanel] prepareSavePanel:MSG_BUTTON_CREATE
                                       message:MSG_PROMPT_CREATE_CRT_PATH
                                      fileName:@"U2FSelfCert" fileTypes:@[@"crt"]];
        [[self toolFilePanel] panelWillCreatePath:sender parentWindow:[self window]];
    }

    - (bool) checkEntries {
        // 入力項目が正しく指定されていない場合は終了
        if ([self checkMustEntry:[self fieldPath] informativeText:MSG_PROMPT_SELECT_CSR_PATH] == false) {
            return false;
        }
        // FIXME:
        // 入力されたファイルパスが存在しない場合は終了
        //
        return true;
    }

    - (bool) checkMustEntry:(NSTextField *)textField informativeText:(NSString *)informativeText {
        // 入力項目が正しく指定されていない場合はfalseを戻す
        if ([[textField stringValue] length] == 0) {
            [ToolPopupWindow warning:MSG_INVALID_FIELD informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath {
        // ファイル選択パネルで選択されたファイルパスを表示する
        [[self fieldPath] setStringValue:filePath];
        [[self fieldPath] becomeFirstResponder];
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath {
        // ファイル保存パネルで選択された出力先ファイルパスを保持し、画面を閉じる
        [[self parameter] setOutPath:filePath];
        [self terminateWindow:NSModalResponseOK];
    }

@end
