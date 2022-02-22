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
    // PIN番号管理用パラメーターを保持
    @property (nonatomic) NSString                     *currentPin;
    @property (nonatomic) NSString                     *renewalPin;
    @property (nonatomic) NSString                     *pinCommandName;

@end

@interface ToolPGPCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;

    - (void)commandWillPerformPGPProcess:(Command)command withParameter:(ToolPGPParameter *)parameter;
    - (void)commandDidResetFirmware:(bool)success;

    - (NSString *)getPGPStatusInfoString;

@end

#endif /* ToolPGPCommand_h */
