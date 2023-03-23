//
//  OATHAccountCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/17.
//
#ifndef OATHAccountCommand_h
#define OATHAccountCommand_h

@interface OATHAccountCommand : NSObject

    - (bool)isUSBCCIDCanConnect;
    - (void)doSelectApplicationForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountAddForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountListForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountDeleteForTarget:(id)object forSelector:(SEL)selector;
    - (void)doCalculateForTarget:(id)object forSelector:(SEL)selector;

@end

#endif /* OATHAccountCommand_h */
