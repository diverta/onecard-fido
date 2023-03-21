//
//  AccountSelectWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/20.
//
#import "AccountSelectWindow.h"
#import "AppCommonMessage.h"
#import "OATHCommand.h"

@interface AccountSelectWindow () <NSTableViewDelegate>

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                 *parentWindow;
    // 画面項目を保持
    @property (assign) IBOutlet NSButton           *buttonSelect;
    // パラメーターの参照を保持
    @property (assign) OATHCommandParameter        *commandParameter;
    // コマンド完了後に継続される処理を保持
    @property (nonatomic) id                        targetForContinue;
    @property (nonatomic) SEL                       selectorForContinue;
    // 画面項目（タイトル／キャプション／アカウント一覧）に表示するデータを保持
    @property (nonatomic) NSString                 *titleString;
    @property (nonatomic) NSString                 *captionString;
    @property (nonatomic) NSArray<NSDictionary *>  *accountArray;

@end

@implementation AccountSelectWindow

    - (void)windowDidLoad {
        // 画面項目の初期化
        [super windowDidLoad];
        [self initFieldValue];
    }

    - (void)initFieldValue {
        // 選択ボタンを使用不可
        [[self buttonSelect] setEnabled:false];
    }

    - (IBAction)buttonSelectDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // このウィンドウを終了
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - For AccountSelectWindow open/close

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent ForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // パラメーターの参照を保持
        [self setCommandParameter:[[OATHCommand instance] parameter]];
        // 親画面の参照を保持
        [self setParentWindow:parent];
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // すでにダイアログがロード済みの場合は、画面項目を再度初期化
        if ([self isWindowLoaded]) {
            [self initFieldValue];
        }
        // タイトル、キャプションを表示
        if ([[self commandParameter] command] == COMMAND_OATH_SHOW_PASSWORD) {
            [self setTitleString:MSG_TITLE_OATH_ACCOUNT_SEL_FOR_TOTP];
            [self setCaptionString:MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_TOTP];
        }
        if ([[self commandParameter] command] == COMMAND_OATH_DELETE_ACCOUNT) {
            [self setTitleString:MSG_TITLE_OATH_ACCOUNT_SEL_FOR_DELETE];
            [self setCaptionString:MSG_CAPTION_OATH_ACCOUNT_SEL_FOR_DELETE];
        }
        // アカウント一覧を表示
        NSMutableArray<NSDictionary *> *array = [[NSMutableArray alloc] init];
        for (NSString *account in [[self commandParameter] accountList]) {
            [array addObject:@{@"account": account}];
        }
        [self setAccountArray:array];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        AccountSelectWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf windowDidCloseWithModalResponse:response];
        }];
        return true;
    }

    - (void)windowDidCloseWithModalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        // Cancelクリック時は実行機能をクリア
        if (modalResponse == NSModalResponseCancel) {
            [[self commandParameter] setCommand:COMMAND_NONE];
        }
        // 戻り先がある場合は制御を戻す
        if ([self targetForContinue] && [self selectorForContinue]) {
            [[self targetForContinue] performSelector:[self selectorForContinue] withObject:nil afterDelay:0.0];
        }
    }

#pragma mark - For account selection

    - (NSIndexSet *)tableView:(NSTableView *)tableView selectionIndexesForProposedSelection:(NSIndexSet *)proposedSelectionIndexes {
        // 例外抑止
        if ([proposedSelectionIndexes count] != 1 || [proposedSelectionIndexes count] > [[self accountArray] count]) {
            return proposedSelectionIndexes;
        }
        // 選択されたアカウントを保持
        NSDictionary *selectedItem = [[self accountArray] objectAtIndex:[proposedSelectionIndexes firstIndex]];
        [[self commandParameter] setSelectedAccount:[selectedItem objectForKey:@"account"]];
        // 選択ボタンを使用可
        [[self buttonSelect] setEnabled:true];
        return proposedSelectionIndexes;
    }

@end
