//
//  CertreqParamWindow.m
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/20.
//
#import "CertreqParamWindow.h"

@interface CertreqParamWindow ()

@end

@implementation CertreqParamWindow

- (void)windowDidLoad {
    [super windowDidLoad];
}

- (IBAction)buttonOKDidPress:(id)sender {
    NSLog(@"CertreqParamWindow: buttonOKDidPress");
    [NSApp endSheet:[self window] returnCode:NSModalResponseOK];
}

- (IBAction)buttonCancelDidPress:(id)sender {
    NSLog(@"CertreqParamWindow: buttonCancelDidPress");
    [NSApp endSheet:[self window] returnCode:NSModalResponseCancel];
}

@end
