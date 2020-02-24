//
//  DFUStartWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/02/24.
//
#ifndef DFUStartWindow_h
#define DFUStartWindow_h

#import <Cocoa/Cocoa.h>

@interface DFUStartWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

    - (void)setLabelVersion:(NSString *)current updateVersion:(NSString *)update;

@end

#endif /* DFUStartWindow_h */
