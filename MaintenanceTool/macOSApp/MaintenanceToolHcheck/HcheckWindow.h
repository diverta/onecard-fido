//
//  HcheckWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef HcheckWindow_h
#define HcheckWindow_h

#import <Cocoa/Cocoa.h>

@interface HcheckWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* HcheckWindow_h */
