//
//  OATHCCIDCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/17.
//
#ifndef OATHCCIDCommand_h
#define OATHCCIDCommand_h

@interface OATHCCIDCommand : NSObject

    - (bool)isUSBCCIDCanConnect;
    - (bool)ccidHelperWillConnect;
    - (void)ccidHelperWillDisconnect;

    - (void)doSelectApplicationForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountAddForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountListForTarget:(id)object forSelector:(SEL)selector;
    - (void)doAccountDeleteForTarget:(id)object forSelector:(SEL)selector;
    - (void)doCalculateForTarget:(id)object forSelector:(SEL)selector;

@end

#endif /* OATHCCIDCommand_h */
