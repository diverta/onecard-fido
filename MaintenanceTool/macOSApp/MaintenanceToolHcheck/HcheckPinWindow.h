//
//  HcheckPinWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef HcheckPinWindow_h
#define HcheckPinWindow_h

#import <Cocoa/Cocoa.h>

@interface HcheckPinWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* HcheckPinWindow_h */
