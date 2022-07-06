//
//  UtilityCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/07/06.
//
#ifndef UtilityCommand_h
#define UtilityCommand_h

@interface UtilityCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)utilityWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* UtilityCommand_h */
