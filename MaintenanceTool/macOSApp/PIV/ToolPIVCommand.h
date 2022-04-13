//
//  ToolPIVCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVCommand_h
#define ToolPIVCommand_h

#import "ToolCommon.h"

@interface ToolPIVParameter : NSObject

    // 鍵作成用パラメーターを保持
    @property (nonatomic) uint8_t                       keySlotId;
    @property (nonatomic) NSString                     *pkeyPemPath;
    @property (nonatomic) NSString                     *certPemPath;
    @property (nonatomic) NSString                     *authPin;
    // PIN番号管理用パラメーターを保持
    @property (nonatomic) NSString                     *currentPin;
    @property (nonatomic) NSString                     *renewalPin;

@end

@interface ToolPIVCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;

    - (void)commandWillPerformPIVProcess:(Command)command withParameter:(ToolPIVParameter *)parameter;
    - (void)commandDidResetFirmware:(bool)success;

    - (NSString *)getPIVSettingDescriptionString;
    - (bool)checkUSBHIDConnection;

@end

#endif /* ToolPIVCommand_h */
