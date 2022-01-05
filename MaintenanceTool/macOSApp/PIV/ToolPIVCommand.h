//
//  ToolPIVCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVCommand_h
#define ToolPIVCommand_h

#import "ToolCommon.h"

@interface ToolPIVCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;

    - (void)commandWillResetFirmware:(Command)command;
    - (void)commandDidResetFirmware:(bool)success;

    - (void)commandWillImportKey:(Command)command withAuthPinCode:(NSString *)pinCodeCur withImporterRef:(id)importer;
    - (void)commandWillChangePin:(Command)command withNewPinCode:(NSString *)pinCodeNew withAuthPinCode:(NSString *)pinCodeCur;
    - (void)commandWillReset:(Command)command;
    - (void)commandWillSetCHUIDAndCCC:(Command)command withImporterRef:(id)importer;
    - (void)commandWillStatus:(Command)command;

    - (NSString *)getPIVSettingDescriptionString;

@end

#endif /* ToolPIVCommand_h */
