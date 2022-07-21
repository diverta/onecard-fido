//
//  FirmwareResetCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/21.
//
#ifndef FirmwareResetCommand_h
#define FirmwareResetCommand_h

#import <Foundation/Foundation.h>

@protocol FirmwareResetCommandDelegate;

@interface FirmwareResetCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestFirmwareReset;

@end

@protocol FirmwareResetCommandDelegate <NSObject>

    - (void)FirmwareResetDidCompleted:(bool)success message:(NSString *)message;

@end

#endif /* FirmwareResetCommand_h */
