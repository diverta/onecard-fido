//
//  ChromeSetupWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/06/04.
//
#import "ChromeSetupWindow.h"
#import "ToolCommonMessage.h"
#import "ToolPopupWindow.h"

@interface ChromeSetupWindow ()

    @property (assign) IBOutlet NSPopUpButton *fieldExtensionType;
    @property (assign) IBOutlet NSTextField   *fieldExtensionID;
    @property (assign) IBOutlet NSTextField   *fieldDescription;

    @property (nonatomic) NSArray<NSString *> *extensionTypeList;
    @property (nonatomic) NSArray<NSString *> *extensionIDList;
    @property (nonatomic) NSArray<NSString *> *extensionDescList;

@end

@implementation ChromeSetupWindow

- (void)windowDidLoad {
    [super windowDidLoad];

    // エクステンションのリストを初期化し、画面項目を初期化
    [self setExtensionTypeList:@[MSG_CHROMEEXT_TYPE_NONE,
                                 MSG_CHROMEEXT_TYPE_GOOGLEDEMOSITE,
                                 MSG_CHROMEEXT_TYPE_LOCALTESTSERVER]];
    [self setExtensionIDList:@[  MSG_CHROMEEXT_ID_NONE,
                                 MSG_CHROMEEXT_ID_GOOGLEDEMOSITE,
                                 MSG_CHROMEEXT_ID_LOCALTESTSERVER]];
    [self setExtensionDescList:@[MSG_CHROMEEXT_DESC_NONE,
                                 MSG_CHROMEEXT_DESC_GOOGLEDEMOSITE,
                                 MSG_CHROMEEXT_DESC_LOCALTESTSERVER]];
    [self initFieldValue];
    NSLog(@"ChromeSetupWindow loaded.");
}

- (void)initFieldValue {
    // エクステンションのリストを総入れ替え
    [[self fieldExtensionType] removeAllItems];
    [[self fieldExtensionType] addItemsWithTitles:[self extensionTypeList]];
    [self extensionTypeSelected];
}

- (void)extensionTypeSelected {
    // 現在選択されているエクステンションに対応するID、説明を表示
    NSUInteger selected = [[self fieldExtensionType] indexOfSelectedItem];
    [[self fieldExtensionID] setStringValue:[[self extensionIDList] objectAtIndex:selected]];
    [[self fieldDescription] setStringValue:[[self extensionDescList] objectAtIndex:selected]];
}

- (IBAction)fieldExtensionTypeDidSelect:(id)sender {
    [self extensionTypeSelected];
}

- (IBAction)buttonOKDidPress:(id)sender {
    [self doProcess:sender];
}

- (IBAction)buttonCancelDidPress:(id)sender {
    [self terminateWindow:NSModalResponseCancel];
}

- (void)terminateWindow:(NSModalResponse)response {
    // エクステンションIDを未選択状態とする
    [self setExtensionID:@""];
    // 画面項目を初期化し、この画面を閉じる
    [self initFieldValue];
    [[self parentWindow] endSheet:[self window] returnCode:response];
}

#pragma mark - Check for entries and process

- (void) doProcess:(id)sender {
    // 未選択の場合は処理中断
    NSUInteger selected = [[self fieldExtensionType] indexOfSelectedItem];
    if (selected == 0) {
        [ToolPopupWindow warning:@"使用するエクステンションを選択してください。" informativeText:nil];
        return;
    }
    // 処理続行を確認
    if ([ToolPopupWindow promptYesNo:MSG_SETUP_CHROME
                     informativeText:MSG_PROMPT_SETUP_CHROME] == false) {
        return;
    }
    // 選択されたエクステンションIDを保持し、この画面を閉じる
    [self setExtensionID:[[self extensionIDList] objectAtIndex:selected]];
    [self terminateWindow:NSModalResponseOK];
}

@end
