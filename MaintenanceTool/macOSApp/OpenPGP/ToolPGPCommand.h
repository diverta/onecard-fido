//
//  ToolPGPCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/12/16.
//
#ifndef ToolPGPCommand_h
#define ToolPGPCommand_h

#import "ToolCommon.h"

@interface ToolPGPParameter : NSObject

    // 鍵作成用パラメーターを保持
    @property (nonatomic) NSString                     *realName;
    @property (nonatomic) NSString                     *mailAddress;
    @property (nonatomic) NSString                     *comment;
    @property (nonatomic) NSString                     *passphrase;
    @property (nonatomic) NSString                     *pubkeyFolderPath;
    @property (nonatomic) NSString                     *backupFolderPath;

@end

@interface ToolPGPCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;

    - (void)commandWillResetFirmware:(Command)command;
    - (void)commandDidResetFirmware:(bool)success;

    - (void)commandWillInstallPGPKey:(id)sender parameter:(ToolPGPParameter *)parameter;
    - (void)commandWillPGPStatus:(id)sender;
    - (void)commandWillPGPReset:(id)sender;

    - (NSString *)getPGPStatusInfoString;

@end

#endif /* ToolPGPCommand_h */
