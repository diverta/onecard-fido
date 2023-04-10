//
//  OATHWindowUtil.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/21.
//
#import "OATHCommand.h"
#import "OATHWindowUtil.h"
#import "ToolPopupWindow.h"
#import "ToolProcessingWindow.h"

@interface OATHWindowUtil ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow             *parentWindow;
    // コマンドクラス、パラメーターの参照を保持
    @property (nonatomic) OATHCommand          *oathCommand;
    @property (nonatomic) OATHCommandParameter *commandParameter;
    // コマンド完了後に継続される処理を保持
    @property (nonatomic) id                    targetForContinue;
    @property (nonatomic) SEL                   selectorForContinue;

@end

@implementation OATHWindowUtil

    - (void)commandWillPerformForTarget:(id)object forSelector:(SEL)selector withParentWindow:(NSWindow *)parentWindow {
        // コールバックを保持
        [self setTargetForContinue:object];
        [self setSelectorForContinue:selector];
        // 進捗画面を表示
        [self setParentWindow:parentWindow];
        [[ToolProcessingWindow defaultWindow] windowWillOpenWithCommandRef:self withParentWindow:[self parentWindow]];
        // コマンドを実行
        [self setOathCommand:[OATHCommand instance]];
        [self setCommandParameter:[[self oathCommand] parameter]];
        [[self oathCommand] commandWillPerformForTarget:self forSelector:@selector(oathProcessDidTerminated)];
    }

    - (void)oathProcessDidTerminated {
        // 進捗画面を閉じる
        [[ToolProcessingWindow defaultWindow] windowWillCloseForTarget:self forSelector:@selector(toolProcessingWindowDidClose)];
    }

    - (void)toolProcessingWindowDidClose {
        // 処理失敗時は、エラーメッセージをポップアップ表示
        if ([[self commandParameter] commandSuccess] == false) {
            NSString *message = [[self commandParameter] resultMessage];
            NSString *informative = [[self commandParameter] resultInformativeMessage];
            [[ToolPopupWindow defaultWindow] critical:message informativeText:informative
                                           withObject:nil forSelector:nil parentWindow:[self parentWindow]];
        }
        // 戻り先がある場合は制御を戻す
        if ([self targetForContinue] && [self selectorForContinue]) {
            [[self targetForContinue] performSelector:[self selectorForContinue] withObject:nil afterDelay:0.0];
        }
    }

@end
