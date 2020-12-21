//
//  ToolInfoWindow.m
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/17.
//
#import "ToolInfoWindow.h"
#import "ToolPIVCommand.h"

// このウィンドウクラスのインスタンスを保持
static ToolInfoWindow *sharedInstance;

@interface ToolInfoWindow ()

    // 親ウィンドウの参照を保持
    @property (nonatomic, weak) NSWindow        *parentWindow;

    @property (assign) IBOutlet NSTextField     *labelTitle;
    @property (assign) IBOutlet NSTextView      *textView;
    @property (assign) IBOutlet NSScrollView    *scrollView;

    // 画面表示する文字列を保持
    @property (nonatomic) NSString *title;
    @property (nonatomic) NSString *info;

@end

@implementation ToolInfoWindow

#pragma mark - Methods for singleton

    + (ToolInfoWindow *)defaultWindow {
        // このクラスのインスタンス化を１度だけ行う
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [[self alloc] initWithWindowNibName:@"ToolInfoWindow"];
        });
        // インスタンスの参照を戻す
        return sharedInstance;
    }

    + (id)allocWithZone:(NSZone *)zone {
        // このクラスのインスタンス化を１度だけ行う
        __block id ret = nil;
        static dispatch_once_t once;
        dispatch_once(&once, ^{
            sharedInstance = [super allocWithZone:zone];
            ret = sharedInstance;
        });
        
        // インスタンスの参照を戻す（２回目以降の呼び出しではnilが戻る）
        return ret;
    }

    - (id)copyWithZone:(NSZone *)zone{
        return self;
    }

#pragma mark - Methods of this instance

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

#pragma mark - For ToolInfoWindow open/close

    - (bool)windowWillOpenWithCommandRef:(id)ref withParentWindow:(NSWindow *)parent
                             titleString:(NSString *)title infoString:(NSString *)info
                     {
        // すでにダイアログが開いている場合は終了
        [self setParentWindow:parent];
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
    }

@end
