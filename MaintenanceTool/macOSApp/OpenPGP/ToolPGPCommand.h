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
    - (void)commandWillResetFirmware:(Command)command;
    - (void)commandDidResetFirmware:(bool)success;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;
    - (void)installPGPKeyWillStart:(id)sender
        realName:(NSString *)realName mailAddress:(NSString *)mailAddress comment:(NSString *)comment
        passphrase:(NSString *)passphrase
        pubkeyFolderPath:(NSString *)pubkeyFolder
        backupFolderPath:(NSString *)backupFolder;
    - (void)pgpStatusWillStart:(id)sender;
    - (void)pgpResetWillStart:(id)sender;
    - (NSString *)pgpStatusInfoString;

@end

#endif /* ToolPGPCommand_h */
