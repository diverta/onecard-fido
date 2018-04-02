//
//  ToolCommand.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2017/11/20.
//
#ifndef ToolCommand_h
#define ToolCommand_h

// for OpenSSL
#import "ToolFileMenu.h"
#import "ToolCommon.h"

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

@end

@protocol ToolCommandDelegate <NSObject>

    - (void)notifyToolCommandMessage:(NSString *)message;
    - (void)notifyToolCommandEnd;

    - (void)toolCommandDidCreateBleRequest;
    - (void)toolCommandDidReceive:(NSDictionary *)u2fResponseDict result:(bool)result;

@end

#endif /* ToolCommand_h */
