//
//  CertreqParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import "CertreqParamWindow.h"
#import "ToolFilePanel.h"

// 画面で使用する文言
#define MSG_INVALID_FIELD       @"入力項目が不正です。"
#define MSG_PROMPT_SELECT_PEM   @"証明書要求時に使用する秘密鍵ファイル(PEM)を選択してください。"
#define MSG_PROMPT_CREATE_CSR   @"作成する証明書要求ファイル(CSR)名を指定してください。"
#define MSG_BUTTON_SELECT       @"選択"
#define MSG_BUTTON_CREATE       @"作成"
#define MSG_PROMPT_INPUT_CN     @"実際に接続されるURLのFQDN（例：www.diverta.co.jp）を入力してください。"
#define MSG_PROMPT_INPUT_O      @"申請組織の名称（例：Diverta Inc.）を入力してください。"
#define MSG_PROMPT_INPUT_L      @"申請組織の事業所住所の市区町村名（例：Shinjuku-ku、Yokohama-shi等）を入力してください。"
#define MSG_PROMPT_INPUT_ST     @"申請組織の事業所住所の都道府県名（例：Tokyo、Kanagawa）を入力してください。"
#define MSG_PROMPT_INPUT_C      @"申請組織の事業所住所の国名（例：JP）を入力してください。"

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
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT message:MSG_PROMPT_SELECT_PEM
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
        [[self toolFilePanel] prepareSavePanel:MSG_BUTTON_CREATE message:MSG_PROMPT_CREATE_CSR
                                      fileName:@"U2FCertReq" fileTypes:@[@"csr"]];
        [[self toolFilePanel] panelWillCreatePath:sender parentWindow:[self window]];
    }

    - (bool) checkEntries {
        // 入力項目が正しく指定されていない場合は終了
        if ([self checkMustEntry:[self fieldPath] informativeText:MSG_PROMPT_SELECT_PEM] == false) {
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
            [self popupMessage:MSG_INVALID_FIELD informativeText:informativeText];
            [textField becomeFirstResponder];
            return false;
        }
        return true;
    }

    - (void) popupMessage:(NSString *)message informativeText:(NSString *)subMessage {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert setAlertStyle:NSAlertStyleWarning];
        [alert setMessageText:message];
        [alert setInformativeText:subMessage];
        [alert runModal];
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
