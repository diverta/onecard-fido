//
//  HcheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef HcheckCommand_h
#define HcheckCommand_h

#import "AppCommand.h"

@interface HcheckCommand : AppCommand

    - (void)hcheckWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* HcheckCommand_h */
