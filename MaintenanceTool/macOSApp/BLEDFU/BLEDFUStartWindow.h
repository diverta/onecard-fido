//
//  BLEDFUStartWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/18.
//
#ifndef BLEDFUStartWindow_h
#define BLEDFUStartWindow_h

#import <Cocoa/Cocoa.h>

@interface BLEDFUStartWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

    - (void)setWindowParameter:(id)toolBLEDFUCommandRef currentVersion:(NSString *)current updateVersion:(NSString *)update;

@end

#endif /* BLEDFUStartWindow_h */
