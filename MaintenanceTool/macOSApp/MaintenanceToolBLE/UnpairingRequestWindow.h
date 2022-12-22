//
//  UnpairingRequestWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/16.
//
#ifndef UnpairingRequestWindow_h
#define UnpairingRequestWindow_h

#import <Cocoa/Cocoa.h>

@interface UnpairingRequestWindow : NSWindowController

    // 親画面の参照を保持
    @property (nonatomic) NSWindow *parentWindow;

    - (void)setParentWindowRef:(id)ref;
    - (void)commandDidStartUnpairingRequestProcessForTarget:(id)target forSelector:(SEL)selector withProgressMax:(int)progressMax;
    - (void)commandDidStartWaitingForUnpairWithDeviceName:(NSString *)deviceName;
    - (void)commandDidNotifyProcessWithMessage:(NSString *)message withProgress:(int)progress;
    - (void)commandDidCancelUnpairingRequestProcess;
    - (void)commandDidTerminateUnpairingRequestProcess:(bool)success;

@end

#endif /* UnpairingRequestWindow_h */
