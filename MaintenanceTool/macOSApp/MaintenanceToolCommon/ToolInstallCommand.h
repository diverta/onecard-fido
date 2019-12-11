//
//  ToolInstallCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/20.
//
#ifndef ToolInstallCommand_h
#define ToolInstallCommand_h

#import "ToolCommon.h"

@interface ToolInstallCommand : NSObject

    - (bool)extractKeyAgreement:(NSData *)keyAgreementResponse;
    - (NSData *)generateSkeyCertInstallCbor:(NSData *)skeyCertBinaryData;
    - (NSData *)generateEraseSkeyCertMessage:(Command)command;
    - (NSData *)extractSkeyCertBinaryData:(Command)command
                                  skeyFilePath:(NSString *)skeyFilePath
                                  certFilePath:(NSString *)certFilePath;

    @property (nonatomic) NSString *lastErrorMessage;

@end

#endif /* ToolInstallCommand_h */
