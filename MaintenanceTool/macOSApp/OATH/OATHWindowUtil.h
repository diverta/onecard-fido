//
//  OATHWindowUtil.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/21.
//
#ifndef OATHWindowUtil_h
#define OATHWindowUtil_h

@interface OATHWindowUtil : NSObject

    - (void)commandWillPerformForTarget:(id)object forSelector:(SEL)selector withParentWindow:(NSWindow *)parentWindow;

@end
#endif /* OATHWindowUtil_h */
