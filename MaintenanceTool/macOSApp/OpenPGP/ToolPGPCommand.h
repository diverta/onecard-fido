//
//  ToolPGPCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#ifndef ToolPGPCommand_h
#define ToolPGPCommand_h

@interface ToolPGPCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;
    - (void)setParametersForGeneratePGPKey:(id)sender
        realName:(NSString *)realName mailAddress:(NSString *)mailAddress comment:(NSString *)comment
        passphrase:(NSString *)passphrase exportFolderPath:(NSString *)exportFolderPath;
    - (void)generatePGPKeyWillStart:(id)sender;

@end

#endif /* ToolPGPCommand_h */
