//
//  OATHAccountCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/17.
//
#ifndef OATHAccountCommand_h
#define OATHAccountCommand_h

@interface OATHAccountCommand : NSObject

    - (void)doAccountAddForTarget:(id)object forSelector:(SEL)selector;

@end

#endif /* OATHAccountCommand_h */
