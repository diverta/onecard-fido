//
//  BLEDFUProcessingWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/21.
//
#ifndef BLEDFUProcessingWindow_h
#define BLEDFUProcessingWindow_h

#import <Cocoa/Cocoa.h>

@interface BLEDFUProcessingWindow : NSWindowController

    @property (nonatomic) NSWindow *parentWindow;

    - (void)commandDidStartDFUProcess;
    - (void)commandDidNotifyDFUProcess:(NSString *)message;
    - (void)commandDidTerminateDFUProcess:(bool)result;

@end

#endif /* BLEDFUProcessingWindow_h */
