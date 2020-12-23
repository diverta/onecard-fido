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
    @property (assign) IBOutlet NSView              *windowView;
    @property (assign) IBOutlet NSTabView           *tabView;
    @property (assign) IBOutlet NSButton            *buttonClose;
    @property (assign) IBOutlet NSButton            *buttonPivStatus;
    @property (assign) IBOutlet NSButton            *buttonInitialSetting;
    @property (assign) IBOutlet NSButton            *buttonClearSetting;

    @property (assign) IBOutlet NSTabViewItem       *tabPkeyCertManagement;
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId1;
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId2;
    @property (assign) IBOutlet NSButton            *buttonPkeySlotId3;
    @property (assign) IBOutlet NSTextField         *fieldPath1;
    @property (assign) IBOutlet NSTextField         *fieldPath2;
    @property (assign) IBOutlet NSButton            *buttonPath1;
    @property (assign) IBOutlet NSButton            *buttonPath2;
    @property (assign) IBOutlet NSTextField         *fieldPin1;
    @property (assign) IBOutlet NSTextField         *fieldPin2;
    @property (assign) IBOutlet NSButton            *buttonInstallPkeyCert;

    @property (assign) IBOutlet NSTabViewItem       *tabPinManagement;
    @property (assign) IBOutlet NSButton            *buttonPinCommand1;
    @property (assign) IBOutlet NSButton            *buttonPinCommand2;
    @property (assign) IBOutlet NSButton            *buttonPinCommand3;
    @property (assign) IBOutlet NSTextField         *labelCurPin;
    @property (assign) IBOutlet NSTextField         *labelNewPin;
    @property (assign) IBOutlet NSTextField         *labelNewPinConf;
    @property (assign) IBOutlet NSTextField         *fieldCurPin;
    @property (assign) IBOutlet NSTextField         *fieldNewPin;
    @property (assign) IBOutlet NSTextField         *fieldNewPinConf;
    @property (assign) IBOutlet NSButton            *buttonPerformPinCommand;

    // 親画面を保持
    @property (nonatomic, weak) NSWindow            *parentWindow;
    // PIV機能処理クラスの参照を保持
    @property (nonatomic, weak) ToolPIVCommand      *toolPIVCommand;
    @property (nonatomic) ToolFilePanel             *toolFilePanel;
    // ラジオボタンで選択中の情報を保持
    @property (nonatomic) uint8_t                    selectedPkeySlotId;
    @property (nonatomic) Command                    selectedPinCommand;

@end

@implementation PIVPreferenceWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
        // 画面項目を初期化
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 最初のタブを選択させる
        [[self tabView] selectTabViewItemAtIndex:0];
        // タブを初期化
        [self initTabPkeyCertManagement];
        [self initTabPinManagement];
    }

    - (void)initTabPkeyCertManagement {
        // ラジオボタンの初期化
        [self initButtonPkeySlotIdsWithDefault:[self buttonPkeySlotId1]];
        // テキストボックスの初期化
        [[self fieldPath1] setStringValue:@""];
        [[self fieldPath2] setStringValue:@""];
        [[self fieldPin1] setStringValue:@""];
        [[self fieldPin2] setStringValue:@""];
        // テキストボックスのカーソルを配置
        [[self fieldPath1] becomeFirstResponder];
    }

    - (void)initTabPinManagement {
        // ラジオボタンの初期化
        [self initButtonPinCommandsWithDefault:[self buttonPinCommand1]];
        // テキストボックスの初期化
        [[self fieldCurPin] setStringValue:@""];
        [[self fieldNewPin] setStringValue:@""];
        [[self fieldNewPinConf] setStringValue:@""];
    }

    - (void)enableButtons:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonClose] setEnabled:enabled];
        [[self buttonPivStatus] setEnabled:enabled];
        [[self buttonInitialSetting] setEnabled:enabled];
        [[self buttonClearSetting] setEnabled:enabled];
        // 現在選択中のタブ内も同様に制御を行う
        NSTabViewItem *item = [[self tabView] selectedTabViewItem];
        if (item == [self tabPkeyCertManagement]) {
            [self enableButtonsInTabPkeyCertManagement:enabled];
        }
        if (item == [self tabPinManagement]) {
            [self enableButtonsInTabPinManagement:enabled];
        }
    }

    - (void)enableButtonsInTabPkeyCertManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonPkeySlotId1] setEnabled:enabled];
        [[self buttonPkeySlotId2] setEnabled:enabled];
        [[self buttonPkeySlotId3] setEnabled:enabled];
        [[self fieldPath1] setEnabled:enabled];
        [[self fieldPath2] setEnabled:enabled];
        [[self buttonPath1] setEnabled:enabled];
        [[self buttonPath2] setEnabled:enabled];
        [[self fieldPin1] setEnabled:enabled];
        [[self fieldPin2] setEnabled:enabled];
        [[self buttonInstallPkeyCert] setEnabled:enabled];
    }

    - (void)enableButtonsInTabPinManagement:(bool)enabled {
        // ボタンや入力欄の使用可能／不可制御
        [[self buttonPinCommand1] setEnabled:enabled];
        [[self buttonPinCommand2] setEnabled:enabled];
        [[self buttonPinCommand3] setEnabled:enabled];
        [[self fieldCurPin] setEnabled:enabled];
        [[self fieldNewPin] setEnabled:enabled];
        [[self fieldNewPinConf] setEnabled:enabled];
        [[self buttonPerformPinCommand] setEnabled:enabled];
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
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
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

    - (void)toolPIVCommandDidProcess:(Command)command withResult:(bool)result {
        [self enableButtons:true];
        switch (command) {
            case COMMAND_CCID_PIV_STATUS:
                // PIV設定情報を、情報表示画面に表示
                [self openToolInfoWindowWithDescription];
                break;
            case COMMAND_CCID_PIV_SET_CHUID:
                [self displayResultMessage:result withName:MSG_PIV_INITIAL_SETTING];
                break;
            case COMMAND_CCID_PIV_RESET:
                [self displayResultMessage:result withName:MSG_PIV_CLEAR_SETTING];
                break;
            case COMMAND_CCID_PIV_IMPORT_KEY:
            case COMMAND_CCID_PIV_CHANGE_PIN:
            case COMMAND_CCID_PIV_CHANGE_PUK:
            case COMMAND_CCID_PIV_UNBLOCK_PIN:
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

    - (void)displayResultMessage:(bool)result withName:(NSString *)name {
        // メッセージをポップアップ表示
        NSString *str = [NSString stringWithFormat:MSG_FORMAT_END_MESSAGE, name,
                         result ? MSG_SUCCESS:MSG_FAILURE];
        if (result) {
            [ToolPopupWindow informational:str informativeText:nil];
        } else {
            [ToolPopupWindow critical:str informativeText:nil];
        }
    }

#pragma mark - 鍵・証明書管理タブ関連

    - (void)initButtonPkeySlotIdsWithDefault:(NSButton *)defaultButton {
        // 「インストールする鍵・証明書」のラジオボタン「PIV認証用」を選択状態にする
        [defaultButton setState:NSControlStateValueOn];
        [self getSelectedPkeySlotIdValue:defaultButton];
    }

    - (void)getSelectedPkeySlotIdValue:(NSButton *)button {
        if (button == [self buttonPkeySlotId1]) {
            [self setSelectedPkeySlotId:0x9a];
        } else if (button == [self buttonPkeySlotId2]) {
            [self setSelectedPkeySlotId:0x9c];
        } else if (button == [self buttonPkeySlotId3]) {
            [self setSelectedPkeySlotId:0x9d];
        } else {
            [self setSelectedPkeySlotId:0x00];
        }
    }

#pragma mark - PIN番号管理タブ関連

    - (void)initButtonPinCommandsWithDefault:(NSButton *)defaultButton {
        // 「実行する機能」のラジオボタン「PIN番号を変更」を選択状態にする
        [defaultButton setState:NSControlStateValueOn];
        [self getSelectedPinCommandValue:defaultButton];
    }

    - (void)getSelectedPinCommandValue:(NSButton *)button {
        // ラジオボタンの選択状態に応じ、入力欄のキャプションも変更する
        if (button == [self buttonPinCommand1]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_CHANGE_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PIN];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PIN];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PIN_FOR_CONFIRM];
        } else if (button == [self buttonPinCommand2]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_CHANGE_PUK];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PUK];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PUK];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PUK_FOR_CONFIRM];
        } else if (button == [self buttonPinCommand3]) {
            [self setSelectedPinCommand:COMMAND_CCID_PIV_UNBLOCK_PIN];
            [[self labelCurPin] setStringValue:MSG_LABEL_CURRENT_PUK];
            [[self labelNewPin] setStringValue:MSG_LABEL_NEW_PIN];
            [[self labelNewPinConf] setStringValue:MSG_LABEL_NEW_PIN_FOR_CONFIRM];
        } else {
            [self setSelectedPinCommand:COMMAND_NONE];
        }
    }

#pragma mark - For file path selection

    - (IBAction)buttonPath1DidPress:(id)sender {
        [self panelWillSelectPath:sender withPrompt:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH];
    }
    - (IBAction)buttonPath2DidPress:(id)sender {
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
    }
    - (void)setFieldPath:(id)sender filePath:(NSString *)filePath WithField:(NSTextField *)field withButton:(NSButton *)button {
        if (sender == button) {
            [field setStringValue:filePath];
            [field setToolTip:filePath];
            [field becomeFirstResponder];
        }
    }

#pragma mark - For install pkey & cert

    - (void)installPkeyCert:(id)sender toKeySlot:(uint8_t)slotId withPkeyField:(NSTextField *)pkeyField withCertField:(NSTextField *)certField  {
        // 入力欄のチェック
        if ([self checkPathEntry:pkeyField messageIfError:MSG_PROMPT_SELECT_PIV_PKEY_PEM_PATH] == false) {
            return;
        }
        if ([self checkPathEntry:certField messageIfError:MSG_PROMPT_SELECT_PIV_CERT_PEM_PATH] == false) {
            return;
        }
        // 事前に確認ダイアログを表示
        if ([ToolPopupWindow promptYesNo:MSG_INSTALL_PIV_PKEY_CERT
                         informativeText:MSG_PROMPT_INSTL_SKEY_CERT] == false) {
            return;
        }
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

@end
