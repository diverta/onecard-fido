//
//  CTAP2HcheckCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/13.
//
#ifndef CTAP2HcheckCommand_h
#define CTAP2HcheckCommand_h

#import <Foundation/Foundation.h>

@protocol CTAP2HcheckCommandDelegate;

@interface CTAP2HcheckCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)doRequestHidCtap2HealthCheck;

@end

@protocol CTAP2HcheckCommandDelegate <NSObject>

    - (void)notifyMessage:(NSString *)message;
    - (void)doResponseCtap2HealthCheck:(bool)success message:(NSString *)message;

@end

#endif /* CTAP2HcheckCommand_h */
