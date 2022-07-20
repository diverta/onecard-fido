//
//  DFUCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/20.
//
#ifndef DFUCommand_h
#define DFUCommand_h

#import "AppCommand.h"

@interface DFUCommand : AppCommand

    - (void)bleDfuProcessWillStart:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* DFUCommand_h */
