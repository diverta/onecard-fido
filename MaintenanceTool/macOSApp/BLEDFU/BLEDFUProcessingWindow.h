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

    - (void)commandDidStartDFUProcess:(id)toolCommandRef;
    - (void)commandDidNotifyDFUProcess:(NSString *)message;
    - (void)commandDidNotifyCancelable:(bool)cancelable;
    - (void)commandDidTerminateDFUProcess:(bool)result;
    - (void)commandDidCancelDFUProcess;

@end

#endif /* BLEDFUProcessingWindow_h */
