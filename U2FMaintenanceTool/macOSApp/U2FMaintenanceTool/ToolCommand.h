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
    COMMAND_CHECK_HEALTH
} Command;

@interface ToolCommand : NSObject

@property (nonatomic) NSArray<NSData *> *commandArray;
@property (nonatomic) NSString *lastOccuredErrorMessage;

@property (nonatomic) Command   command;
@property (nonatomic) NSString *skeyFilePath;
@property (nonatomic) NSString *certFilePath;
@property (nonatomic) bool      commandSuccess;

- (bool)createCommandArray;
- (bool)doWithResponseValue:(NSData *)responseValue;

@end

#endif /* ToolCommand_h */
