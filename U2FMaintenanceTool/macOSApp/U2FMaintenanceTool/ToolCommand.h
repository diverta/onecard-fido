//
//  ToolCommand.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#ifndef ToolCommand_h
#define ToolCommand_h

typedef enum : NSInteger {
    COMMAND_NONE = 1,
    COMMAND_ERASE_BOND,
    COMMAND_ERASE_SKEY_CERT,
    COMMAND_INSTALL_SKEY,
    COMMAND_INSTALL_CERT,
    COMMAND_TEST_REGISTER,
    COMMAND_TEST_AUTH_CHECK,
    COMMAND_TEST_AUTH_NO_USER_PRESENCE,
    COMMAND_TEST_AUTH_USER_PRESENCE
} Command;

@interface ToolCommand : NSObject

@property (nonatomic) NSArray<NSData *> *commandArray;
@property (nonatomic) NSString *lastOccuredErrorMessage;

@property (nonatomic) Command   command;
@property (nonatomic) NSString *skeyFilePath;
@property (nonatomic) NSString *certFilePath;
@property (nonatomic) bool      commandSuccess;

- (bool)createCommandArrayFor:(Command)command fromData:(NSData *)parameterData;
- (bool)doAfterResponseFor:(Command)command withData:(NSData *)responseData;

- (NSData *)generateHexBytesFrom:(NSString *)hexString;

@end

#endif /* ToolCommand_h */
