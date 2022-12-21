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

    - (void)setParentWindowRef:(id)ref;
    - (void)commandDidStartUnpairingRequestProcessForTarget:(id)target forSelector:(SEL)selector withProgressMax:(int)progressMax;
    - (void)commandDidCancelUnpairingRequestProcess;

@end

#endif /* UnpairingRequestWindow_h */
