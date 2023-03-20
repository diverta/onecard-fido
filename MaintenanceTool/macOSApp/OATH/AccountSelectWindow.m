//
//  AccountSelectWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/20.
//
#import "OATHCommand.h"
#import "AccountSelectWindow.h"

@interface AccountSelectWindow ()

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
        // パラメーターの参照を保持
        [self setCommandParameter:[[OATHCommand instance] parameter]];
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

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent withTitle:(NSString *)title withCaption:(NSString *)caption
                                 ForTarget:(id)object forSelector:(SEL)selector {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
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
        [self setTitleString:title];
        [self setCaptionString:caption];
        // TODO: 仮の実装です。（アカウント一覧を表示）
        static int a = 0;
        NSString *string1 = [NSString stringWithFormat:@"sample%d", a++];
        NSString *string2 = [NSString stringWithFormat:@"sample%d", a++];
        [self setAccountArray:@[@{@"account":string1}, @{@"account":string2}]];
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

@end
