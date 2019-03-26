//
//  ToolInstallCommand.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#ifndef ToolInstallCommand_h
#define ToolInstallCommand_h

#import "ToolCommon.h"

@interface ToolInstallCommand : NSObject

    - (NSData *)generateEraseSkeyCertMessage:(Command)command;
    - (NSData *)generateInstallSkeyCertMessage:(Command)command
                                  skeyFilePath:(NSString *)skeyFilePath
                                  certFilePath:(NSString *)certFilePath;

    @property (nonatomic) NSString *lastErrorMessage;

@end

#endif /* ToolInstallCommand_h */
