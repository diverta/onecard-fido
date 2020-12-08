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

    - (void)commandWillImportKey:(Command)command withAuthPinCode:(NSString *)pinCodeCur withImporter:(ToolPIVImporter *)importer;
    - (void)commandWillChangePin:(Command)command withNewPinCode:(NSString *)pinCodeNew withAuthPinCode:(NSString *)pinCodeCur;
    - (void)commandWillReset:(Command)command;
    - (void)commandWillSetCHUIDAndCCC:(Command)command withImporter:(ToolPIVImporter *)importer;
    - (void)commandWillStatus:(Command)command;
    - (void)ccidHelperDidProcess:(bool)success response:(NSData *)resp status:(uint16_t)sw;

@end

#endif /* ToolPIVCommand_h */
