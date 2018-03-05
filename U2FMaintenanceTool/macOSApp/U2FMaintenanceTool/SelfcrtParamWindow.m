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
#import "ToolParamWindow.h"

@interface SelfcrtParamWindow () <ToolFilePanelDelegate>

    @property (assign) IBOutlet NSTextField *fieldPath;
    @property (assign) IBOutlet NSTextField *fieldPemPath;
    @property (assign) IBOutlet NSTextField *fieldDays;
    @property (assign) IBOutlet NSButton    *buttonPath;
    @property (assign) IBOutlet NSButton    *buttonPemPath;

    @property (nonatomic) ToolFilePanel *toolFilePanel;

@end

@implementation SelfcrtParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        [self setParameter:[[SelfCertParameter alloc] init]];
        
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 画面項目を初期値に設定
        [[self fieldPath]    setStringValue:@""];
        [[self fieldPemPath] setStringValue:@""];
        [[self fieldDays]    setStringValue:@"365"];

        // 最初の項目にフォーカス
        [[self fieldPath] becomeFirstResponder];
    }

    - (IBAction)buttonFieldPathPress:(id)sender {
        // ファイル選択パネルをモーダル表示
        [[self toolFilePanel] prepareOpenPanel:MSG_BUTTON_SELECT
                                       message:MSG_PROMPT_SELECT_CSR_PATH
                                     fileTypes:@[@"csr"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)buttonFieldPathPemPress:(id)sender {
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
        // 画面入力値をパラメーターに保持
        [[self parameter] setCsrPath:[[self fieldPath] stringValue]];
        [[self parameter] setPemPath:[[self fieldPemPath] stringValue]];
        [[self parameter] setDays:   [[self fieldDays] stringValue]];
        
        // 画面項目を初期化し、この画面を閉じる
        [self initFieldValue];
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
        if ([ToolParamWindow checkMustEntry:[self fieldPath]
                            informativeText:MSG_PROMPT_SELECT_CSR_PATH] == false) {
            return false;
        }
        if ([ToolParamWindow checkMustEntry:[self fieldPemPath]
                            informativeText:MSG_PROMPT_SELECT_PEM_PATH] == false) {
            return false;
        }
        if ([ToolParamWindow checkMustEntry:[self fieldDays]
                            informativeText:MSG_PROMPT_INPUT_CRT_DAYS] == false) {
            return false;
        }
        if ([ToolParamWindow checkIsNumber:[self fieldDays]
                            informativeText:MSG_PROMPT_INPUT_CRT_DAYS] == false) {
            return false;
        }
        // 入力されたファイルパスが存在しない場合は終了
        if ([ToolParamWindow checkFileExist:[self fieldPath]
                            informativeText:MSG_PROMPT_EXIST_CSR_PATH] == false) {
            return false;
        }
        if ([ToolParamWindow checkFileExist:[self fieldPemPath]
                            informativeText:MSG_PROMPT_EXIST_PEM_PATH] == false) {
            return false;
        }
        return true;
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath
                 modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示
        if (modalResponse == NSFileHandlingPanelOKButton) {
            if (sender == [self buttonPath]) {
                [[self fieldPath] setStringValue:filePath];
                [[self fieldPath] becomeFirstResponder];
            }
            if (sender == [self buttonPemPath]) {
                [[self fieldPemPath] setStringValue:filePath];
                [[self fieldPemPath] becomeFirstResponder];
            }
        }
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath
                 modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル保存パネルで選択された出力先ファイルパスを保持
        if (modalResponse == NSFileHandlingPanelOKButton) {
            [[self parameter] setOutPath:filePath];
            [self terminateWindow:NSModalResponseOK];
        }
    }

@end
