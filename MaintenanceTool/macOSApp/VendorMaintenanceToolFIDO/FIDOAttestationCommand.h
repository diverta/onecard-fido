//
//  FIDOAttestationCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/11.
//
#ifndef FIDOAttestationCommand_h
#define FIDOAttestationCommand_h

#import <Foundation/Foundation.h>

@protocol FIDOAttestationCommandDelegate;

@interface FIDOAttestationCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestInstallAttestation:(id)commandParameterRef;
    - (void)doRequestRemoveAttestation;

@end

@protocol FIDOAttestationCommandDelegate <NSObject>

    - (void)FIDOAttestationCommandDidCompleted:(bool)success message:(NSString *)message;

@end

#endif /* FIDOAttestationCommand_h */
