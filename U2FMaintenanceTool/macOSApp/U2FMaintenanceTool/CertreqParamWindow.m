//
//  CertreqParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import "CertreqParamWindow.h"
#import "ToolFilePanel.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"

@interface CertreqParamWindow () <ToolFilePanelDelegate>

    @property (nonatomic) ToolFilePanel *toolFilePanel;

@end

@implementation CertreqParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        [[self fieldC] setStringValue:@"JP"];
    }

    - (IBAction)buttonFieldPathPress:(id)sender {
        // ファイル選択パネルをモーダル表示
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT
                                       message:MSG_PROMPT_SELECT_PEM_PATH
                                     fileTypes:@[@"pem"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        [self doProcess:sender];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Check for entries and process

    - (void) doProcess:(id)sender {
        // 入力内容チェックがOKであれば、ファイル保存パネルをモーダル表示
        if ([self checkEntries] == false) {
            return;
        }
        [[self toolFilePanel] prepareSavePanel:MSG_BUTTON_CREATE
                                       message:MSG_PROMPT_CREATE_CSR_PATH
                                      fileName:@"U2FCertReq" fileTypes:@[@"csr"]];
        [[self toolFilePanel] panelWillCreatePath:sender parentWindow:[self window]];
    }

    - (bool) checkEntries {
        // 入力項目が正しく指定されていない場合は終了
        if ([self checkMustEntry:[self fieldPath] informativeText:MSG_PROMPT_SELECT_PEM_PATH] == false) {
            return false;
        }
        if ([self checkMustEntry:[self fieldCN] informativeText:MSG_PROMPT_INPUT_CN] == false) {
            return false;
        }
        if ([self checkMustEntry:[self fieldO] informativeText:MSG_PROMPT_INPUT_O] == false) {
            return false;
        }
        if ([self checkMustEntry:[self fieldL] informativeText:MSG_PROMPT_INPUT_L] == false) {
            return false;
        }
        if ([self checkMustEntry:[self fieldST] informativeText:MSG_PROMPT_INPUT_ST] == false) {
            return false;
        }
        if ([self checkMustEntry:[self fieldC] informativeText:MSG_PROMPT_INPUT_C] == false) {
            return false;
        }
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
        [self setOutputPath:filePath];
        [self terminateWindow:NSModalResponseOK];
    }

@end
