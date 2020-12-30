//
//  ToolPIVCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVCommand_h
#define ToolPIVCommand_h

#import "ToolCommon.h"
#import "ToolPIVImporter.h"

@interface ToolPIVCommand : NSObject
    // エラーメッセージテキストを保持
    @property (nonatomic) NSString *lastErrorMessage;

    - (id)initWithDelegate:(id)delegate;
    - (void)commandWillOpenPreferenceWindowWithParent:(NSWindow *)parent;
    - (void)commandDidClosePreferenceWindow;

    - (void)commandWillImportKey:(Command)command withAuthPinCode:(NSString *)pinCodeCur withImporter:(ToolPIVImporter *)importer;
    - (void)commandWillChangePin:(Command)command withNewPinCode:(NSString *)pinCodeNew withAuthPinCode:(NSString *)pinCodeCur;
    - (void)commandWillReset:(Command)command;
    - (void)commandWillSetCHUIDAndCCC:(Command)command withImporter:(ToolPIVImporter *)importer;
    - (void)commandWillStatus:(Command)command;

    - (NSString *)getPIVSettingDescriptionString;

@end

#endif /* ToolPIVCommand_h */
