//
//  FIDOAttestationCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/22.
//
#ifndef FIDOAttestationCommand_h
#define FIDOAttestationCommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol FIDOAttestationCommandDelegate;

@interface FIDOAttestationCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (NSData *)generatedInstallMessage;
    - (bool)generateInstallMessageFrom:(NSArray<NSString *> *)selectedFilePaths;

@end

@protocol FIDOAttestationCommandDelegate <NSObject>

    - (void)notifyErrorMessage:(NSString *)message;

@end

#endif /* FIDOAttestationCommand_h */
