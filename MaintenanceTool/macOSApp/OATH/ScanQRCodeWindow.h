//
//  ScanQRCodeWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/14.
//
#ifndef ScanQRCodeWindow_h
#define ScanQRCodeWindow_h

#import <Cocoa/Cocoa.h>

@interface ScanQRCodeWindow : NSWindowController

    - (bool)windowWillOpenWithParentWindow:(NSWindow *)parent;

@end

#endif /* ScanQRCodeWindow_h */
