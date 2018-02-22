//
//  CertreqParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import "CertreqParamWindow.h"
#import "ToolFilePanel.h"

@interface CertreqParamWindow () <ToolFilePanelDelegate>

    @property (nonatomic) ToolFilePanel *toolFilePanel;

@end

@implementation CertreqParamWindow

    - (void)windowDidLoad {
        [super windowDidLoad];
        
        [self setToolFilePanel:[[ToolFilePanel alloc] initWithDelegate:self]];
    }

    - (IBAction)buttonFieldPathPress:(id)sender {
        [[self toolFilePanel] prepareOpenPanel:@"選択" message:@"秘密鍵ファイル(PEM)を選択してください"
                                     fileTypes:@[@"pem"]];
        [[self toolFilePanel] panelWillSelectPath:sender parentWindow:[self window]];
    }

    - (IBAction)buttonOKDidPress:(id)sender {
        NSLog(@"CertreqParamWindow: buttonOKDidPress");
        [self terminateWindow:NSModalResponseOK];
    }

    - (IBAction)buttonCancelDidPress:(id)sender {
        NSLog(@"CertreqParamWindow: buttonCancelDidPress");
        [self terminateWindow:NSModalResponseCancel];
    }

    - (void)terminateWindow:(NSModalResponse)response {
        [[self parentWindow] endSheet:[self window] returnCode:response];
    }

#pragma mark - Call back from ToolFilePanel

    - (void)panelDidSelectPath:(id)sender filePath:(NSString*)filePath {
        // ファイル選択パネルで選択されたファイルパスを表示する
        [[self fieldPath] setStringValue:filePath];
        [[self fieldPath] becomeFirstResponder];
    }

    - (void)panelDidCreatePath:(id)sender filePath:(NSString*)filePath {
    }

@end
