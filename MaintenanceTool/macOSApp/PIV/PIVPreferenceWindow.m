//
//  PIVPreferenceWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/21.
//
#import "PIVPreferenceWindow.h"
#import "ToolFilePanel.h"
#import "ToolInfoWindow.h"
#import "ToolPIVCommand.h"
#import "ToolPopupWindow.h"
#import "ToolCommonMessage.h"
#import "ToolLogFile.h"

@interface PIVPreferenceWindow () <ToolFilePanelDelegate>

    // ファイル選択用のテキストボックス、ボタン
    @property (assign) IBOutlet NSTextField         *fieldPath1;
    @property (assign) IBOutlet NSTextField         *fieldPath2;
    @property (assign) IBOutlet NSTextField         *fieldPath3;
    @property (assign) IBOutlet NSTextField         *fieldPath4;
    @property (assign) IBOutlet NSTextField         *fieldPath5;
    @property (assign) IBOutlet NSTextField         *fieldPath6;
    @property (assign) IBOutlet NSButton            *buttonPath1;
    @property (assign) IBOutlet NSButton            *buttonPath2;
    @property (assign) IBOutlet NSButton            *buttonPath3;
    @property (assign) IBOutlet NSButton            *buttonPath4;
    @property (assign) IBOutlet NSButton            *buttonPath5;
    @property (assign) IBOutlet NSButton            *buttonPath6;

    // 親画面を保持
    @property (nonatomic, weak) NSWindow            *parentWindow;
    // PIV機能処理クラスの参照を保持
    @property (nonatomic, weak) ToolPIVCommand      *toolPIVCommand;
    @property (nonatomic) ToolFilePanel             *toolFilePanel;


@end

@implementation PIVPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
    }

    - (void)initFieldValue {
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

    - (IBAction)buttonPivStatusDidPress:(id)sender {
        // PIV設定情報取得
        [[self toolPIVCommand] commandWillStatus:COMMAND_CCID_PIV_STATUS];
    }

    - (IBAction)buttonCloseDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
    }

#pragma mark - For PIVPreferenceWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref parentWindow:(NSWindow *)parent {
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // ToolPIVCommandの参照を保持
        [self setToolPIVCommand:ref];
        // 画面項目を初期化
        [self initFieldValue];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        PIVPreferenceWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithSender:ref modalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithSender:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        if ([sender isMemberOfClass:[ToolPIVCommand class]]) {
            // ToolPIVCommand経由でメイン画面に制御を戻す
            ToolPIVCommand *command = (ToolPIVCommand *)sender;
            [command commandDidClosePreferenceWindow];
        }
    }

#pragma mark - For ToolPIVCommand functions

    - (void)toolPIVCommandDidProcess:(Command)command {
        switch (command) {
            case COMMAND_CCID_PIV_STATUS:
                // PIV設定情報を、情報表示画面に表示
                [self openToolInfoWindowWithDescription];
                break;
            default:
                break;
        }
    }

    - (void)openToolInfoWindowWithDescription {
        // PIV設定情報を、情報表示画面に表示
        ToolInfoWindow *infoWindow = [ToolInfoWindow defaultWindow];
        ToolPIVCommand *command = [self toolPIVCommand];
        [infoWindow windowWillOpenWithCommandRef:command withParentWindow:[self window]
                                     titleString:PROCESS_NAME_CCID_PIV_STATUS
                                      infoString:[command getPIVSettingDescriptionString]];
    }

#pragma mark - For file path selection

    - (IBAction)buttonPath1DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }
    - (IBAction)buttonPath2DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH];
    }
    - (IBAction)buttonPath3DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }
    - (IBAction)buttonPath4DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH];
    }
    - (IBAction)buttonPath5DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }
    - (IBAction)buttonPath6DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH];
    }
    - (void)panelWillSelectPath:(id)sender withPrompt:(NSString *)prompt {
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]
                                       withPrompt:MSG_BUTTON_SELECT withMessage:prompt withFileTypes:@[@"pem"]];
    }
    - (void)panelDidSelectPath:(id)sender filePath:(NSString *)filePath modalResponse:(NSInteger)modalResponse {
        // OKボタン押下時は、ファイル選択パネルで選択されたファイルパスを表示する
        if (modalResponse != NSFileHandlingPanelOKButton) {
            return;
        }
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath1] withButton:[self buttonPath1]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath2] withButton:[self buttonPath2]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath3] withButton:[self buttonPath3]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath4] withButton:[self buttonPath4]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath5] withButton:[self buttonPath5]];
        [self setFieldPath:sender filePath:filePath WithField:[self fieldPath6] withButton:[self buttonPath6]];
    }
    - (void)setFieldPath:(id)sender filePath:(NSString *)filePath WithField:(NSTextField *)field withButton:(NSButton *)button {
        if (sender == button) {
            [field setStringValue:filePath];
            [field setToolTip:filePath];
            [field becomeFirstResponder];
        }
    }

@end
