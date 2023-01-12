//
//  FIDOAttestationWindow.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/12.
//
#ifndef FIDOAttestationWindow_h
#define FIDOAttestationWindow_h

#import <Cocoa/Cocoa.h>

@interface FIDOAttestationWindow : NSWindowController

    - (void)setParentWindowRef:(id)ref withCommandRef:(id)commandRef withParameterRef:(id)parameterRef;

@end

#endif /* FIDOAttestationWindow_h */
