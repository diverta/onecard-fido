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
    COMMAND_TEST_AUTH_USER_PRESENCE,
    COMMAND_U2F_PROCESS,
    COMMAND_SETUP_CHROME_NATIVE_MESSAGING
} Command;

@protocol ToolCommandDelegate;

    @interface ToolCommand : NSObject

    @property (nonatomic, weak) id<ToolCommandDelegate> delegate;

    @property (nonatomic) Command command;
    @property (nonatomic) NSArray<NSData *> *bleRequestArray;

    - (id)initWithDelegate:(id<ToolCommandDelegate>)delegate;
    - (void)toolCommandWillCreateBleRequest:(Command)command;
    - (void)toolCommandWillProcessBleResponse;
    - (bool)isResponseCompleted:(NSData *)responseData;
    - (void)toolCommandWillSetup:(Command)command;

    - (void)setInstallParameter:(Command)command
                   skeyFilePath:(NSString *)skeyFilePath
                   certFilePath:(NSString *)certFilePath;
    - (void)setU2FProcessParameter:(Command)command
                 bleHelperMessages:(NSArray<NSDictionary *> *)bleHelperMessages;

    - (NSString *)processNameOfCommand;

@end

@protocol ToolCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;

    - (void)toolCommandDidCreateBleRequest;
    - (void)toolCommandDidFail:(NSString *)errorMessage;
    - (void)toolCommandDidSuccess;

    - (void)toolCommandDidReceive:(NSDictionary *)u2fResponseDict;
    - (void)toolCommandDidSetup:(bool)result;

@end

#endif /* ToolCommand_h */
