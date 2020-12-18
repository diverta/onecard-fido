//
//  ToolInfoWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/17.
//
#import "ToolInfoWindow.h"
#import "ToolPIVCommand.h"

@interface ToolInfoWindow ()

    @property (assign) IBOutlet NSTextField     *labelTitle;
    @property (assign) IBOutlet NSTextView      *textView;
    @property (assign) IBOutlet NSScrollView    *scrollView;

    // 画面表示する文字列を保持
    @property (nonatomic) NSString *title;
    @property (nonatomic) NSString *info;

@end

@implementation ToolInfoWindow

    - (void)windowDidLoad {
        // テキストエリアの初期化
        [super windowDidLoad];
        [[self textView] setFont:[NSFont fontWithName:@"Courier" size:12]];
    }

    - (void)initFieldValue {
        // 画面にブランク文字を表示
        [[self labelTitle] setStringValue:@""];
        [[self textView] setString:@""];
    }

    - (void)displayFieldValue {
        // 画面にタイトル、情報を表示
        [[self labelTitle] setStringValue:[self title]];
        [[self textView] setString:[self info]];
    }

    - (void)adjustView {
        // 横スクロールできるように幅を再設定
        NSTextView *view = [self textView];
        NSTextContainer *textContainer = [view textContainer];
        NSSize size = NSMakeSize([view bounds].size.width * 10, [view bounds].size.height + 100);
        [textContainer setContainerSize:size];
        [textContainer setHeightTracksTextView:NO];
        [textContainer setWidthTracksTextView:YES];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        // この画面を閉じる
        if ([self parentWindow]) {
            [[self parentWindow] endSheet:[self window] returnCode:response];
        }
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        [self terminateWindow:NSModalResponseOK];
    }

    - (bool)windowWillOpenWithCommandRef:(id)ref titleString:(NSString *)title infoString:(NSString *)info {
        // すでにダイアログが開いている場合は終了
        if ([[[self parentWindow] sheets] count] > 0) {
            return false;
        }
        // 画面に表示する文字列を保持
        [self initFieldValue];
        [self setTitle:title];
        [self setInfo:info];
        // ダイアログをモーダルで表示
        NSWindow *dialog = [self window];
        ToolInfoWindow * __weak weakSelf = self;
        [[self parentWindow] beginSheet:dialog completionHandler:^(NSModalResponse response){
            // ダイアログが閉じられた時の処理
            [weakSelf toolInfoWindowDidClose:ref modalResponse:response];
        }];
        // 画面項目にテキストを表示
        [self displayFieldValue];
        // 横スクロールできるように幅を再設定
        [self performSelector:@selector(adjustView) withObject:nil afterDelay:0.5];
        return true;
    }

    - (void)toolInfoWindowDidClose:(id)sender modalResponse:(NSInteger)modalResponse {
        // 画面を閉じる
        [self close];
        if ([sender isMemberOfClass:[ToolPIVCommand class]]) {
            // PIV機能コマンドに制御を戻す
            ToolPIVCommand *command = (ToolPIVCommand *)sender;
            [command toolInfoWindowDidClose:self modalResponse:modalResponse];
        }
    }

@end
