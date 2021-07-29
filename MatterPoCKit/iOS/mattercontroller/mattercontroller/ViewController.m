//
//  ViewController.m
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/27.
//
#import "ViewController.h"

#import "MatterCommand.h"
#import "MatterStrings.h"

@interface ViewController () <MatterCommandDelegate>

    @property (assign) IBOutlet UIButton    *buttonPairing;
    @property (assign) IBOutlet UIButton    *buttonUpdateAddress;
    @property (assign) IBOutlet UIButton    *buttonOffCommand;
    @property (assign) IBOutlet UIButton    *buttonOnCommand;
    @property (assign) IBOutlet UITextView  *textViewUsage;
    @property (assign) IBOutlet UITextView  *textViewMessage;

    @property (nonatomic) MatterCommand     *matterCommand;

@end

@implementation ViewController

    - (void)viewDidLoad {
        [super viewDidLoad];
        // Do any additional setup after loading the view.
        [self initializeCommand];
        // Disable command buttons
        [self setButtonDoCommandEnabled:false];
        // Display initial message
        [[self textViewUsage] setText:MSG_USAGE_TEXT_INITIAL];
        [self displayStatusText:MSG_STATUS_TEXT_INITIAL];
    }

#pragma mark - ボタン押下時の処理

    - (IBAction)buttonPairingClicked:(id)sender {
        // ボタンを押下不可に変更
        [self setButtonsEnabled:false];
        [self displayStatusText:msg_pairing_will_start];
        // スキャンを開始
        [[self matterCommand] startBLEConnection:self];
    }

    - (IBAction)buttonUpdateAddressClicked:(id)sender {
    }

    - (IBAction)buttonOffCommandClicked:(id)sender {
    }

    - (IBAction)buttonOnCommandClicked:(id)sender {
    }

# pragma mark - 画面制御

    - (void)setButtonsEnabled:(bool)b {
        [[self buttonPairing] setEnabled:b];
        [self setButtonDoCommandEnabled:b];
    }

    - (void)setButtonDoCommandEnabled:(bool)b {
        [[self buttonOffCommand] setEnabled:b];
        [[self buttonOnCommand] setEnabled:b];
    }

    - (void)displayStatusText:(NSString *)message {
        if (message) {
            // ステータス表示欄に文字列を表示
            [[self textViewMessage] setText:message];
        }
    }

    - (void)appendStatusText:(NSString *)message {
        if (message) {
            // ステータス表示欄に文字列を追加表示
            NSString *s = [[[self textViewMessage] text] stringByAppendingFormat:@"\n%@", message];
            [[self textViewMessage] setText:s];

            // Scroll to the bottom of the content
            NSRange lastLine = NSMakeRange([[[self textViewMessage] text] length] - 1, 1);
            [[self textViewMessage] scrollRangeToVisible:lastLine];
        }
    }

# pragma mark - コマンド関連

    - (void)initializeCommand {
        // コマンドクラスを初期化
        [self setMatterCommand:[[MatterCommand alloc] initWithDelegate:self]];
    }

    - (void)didPairingComplete:(bool)success {
        // すでにペアリング処理が完了している場合は無視
        if ([[self buttonPairing] isEnabled]) {
            return;
        }
        // ボタンを押下可に変更
        [self setButtonsEnabled:true];
        // コマンド実行ボタンを押下可／不可に設定
        [self setButtonDoCommandEnabled:success];
        // ステータス表示欄にメッセージを追加表示
        if (success) {
            [self appendStatusText:msg_pairing_success];
            NSLog(@"Matter device commissioning success");
        } else {
            [self appendStatusText:msg_pairing_failure];
            NSLog(@"Matter device commissioning failed");
        }
    }

    - (void)notifyMessage:(NSString *)message {
        if (message) {
            // ステータス表示欄に文字列を追加表示
            [self appendStatusText:message];
        }
    }

@end
