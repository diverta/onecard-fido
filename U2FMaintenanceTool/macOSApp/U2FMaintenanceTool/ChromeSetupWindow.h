//
//  ChromeSetupWindow.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/06/04.
//
#import <Cocoa/Cocoa.h>

@interface ChromeSetupWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;
    @property (nonatomic) NSString *extensionID;

@end
