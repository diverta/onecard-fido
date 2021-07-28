//
//  ViewController.m
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/27.
//
#import "ViewController.h"

#import "MatterCommand.h"
#import "MatterStrings.h"

@interface ViewController ()

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
        [self setMatterCommand:[[MatterCommand alloc] initWithViewControllerRef:self]];
    }

@end
