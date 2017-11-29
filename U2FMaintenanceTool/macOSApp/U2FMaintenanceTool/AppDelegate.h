#import <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate>

    @property (assign) IBOutlet NSWindow   *window;
    @property (assign) IBOutlet NSButton   *button1;
    @property (assign) IBOutlet NSButton   *button2;
    @property (assign) IBOutlet NSButton   *button3;
    @property (assign) IBOutlet NSButton   *button4;
    @property (assign) IBOutlet NSButton   *buttonQuit;
    @property (assign) IBOutlet NSTextView *textView;

    @property (assign) IBOutlet NSTextField *fieldPath1;
    @property (assign) IBOutlet NSTextField *fieldPath2;
    @property (assign) IBOutlet NSButton    *buttonPath1;
    @property (assign) IBOutlet NSButton    *buttonPath2;

    - (IBAction)button1DidPress:(id)sender;
    - (IBAction)button2DidPress:(id)sender;
    - (IBAction)button3DidPress:(id)sender;
    - (IBAction)button4DidPress:(id)sender;
    - (IBAction)buttonQuitDidPress:(id)sender;

    - (IBAction)buttonPath1DidPress:(id)sender;
    - (IBAction)buttonPath2DidPress:(id)sender;

@end
