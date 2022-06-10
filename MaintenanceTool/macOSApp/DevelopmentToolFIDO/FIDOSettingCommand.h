//
//  FIDOSettingCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/08.
//
#ifndef FIDOSettingCommand_h
#define FIDOSettingCommand_h

@interface FIDOSettingCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)FIDOSettingWindowWillOpen:(id)sender parentWindow:(NSWindow *)parentWindow;

@end

#endif /* FIDOSettingCommand_h */
