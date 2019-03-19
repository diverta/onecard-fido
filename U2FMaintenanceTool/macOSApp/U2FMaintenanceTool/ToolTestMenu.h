//
//  ToolTestMenu.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/19.
//
#ifndef ToolTestMenu_h
#define ToolTestMenu_h

#import "ToolHIDHelper.h"

@interface ToolTestMenu : NSObject

    - (id)initWithHelper:(ToolHIDHelper *)helper;
    - (void)doTestCtapHidInit;
    - (void)doTestBleDummy;

@end

#endif /* ToolTestMenu_h */
