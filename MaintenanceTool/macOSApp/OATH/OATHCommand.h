//
//  OATHCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//

#ifndef OATHCommand_h
#define OATHCommand_h

#import "AppCommand.h"

@interface OATHCommandParameter : NSObject

    @property (nonatomic) Command       command;

@end

@interface OATHCommand : AppCommand

    - (void)oathWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* OATHCommand_h */
