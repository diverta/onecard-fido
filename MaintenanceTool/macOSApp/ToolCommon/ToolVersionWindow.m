//
//  ToolVersionWindow.m
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/07.
//
#import "ToolVersionWindow.h"

@interface ToolVersionWindow ()

    // 親画面の参照を保持
    @property (nonatomic) NSWindow                     *parentWindow;
    // 画面項目の参照を保持
    @property (weak) IBOutlet NSTextField              *labelToolName;
    @property (weak) IBOutlet NSTextField              *labelVersion;
    @property (weak) IBOutlet NSTextField              *labelCopyright;
    // バージョン情報を保持
    @property (nonatomic) NSString                     *toolName;
    @property (nonatomic) NSString                     *version;
    @property (nonatomic) NSString                     *copyright;

@end

@implementation ToolVersionWindow

    - (void)windowDidLoad {
        // 画面項目に設定
        [super windowDidLoad];
        [[self labelToolName] setStringValue:[self toolName]];
        [[self labelVersion] setStringValue:[self version]];
        [[self labelCopyright] setStringValue:[self copyright]];
    }

    - (void)setParentWindowRef:(id)ref {
        // 親画面の参照を保持
        [self setParentWindow:(NSWindow *)ref];
    }

    - (void)setVersionInfoWithToolName:(NSString *)toolName toolVersion:(NSString *)version toolCopyright:(NSString *)copyright {
        // バージョン情報を保持
        [self setToolName:toolName];
        [self setVersion:version];
        [self setCopyright:copyright];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        // この画面を閉じる
        [[self parentWindow] endSheet:[self window] returnCode:NSModalResponseCancel];
    }

@end
