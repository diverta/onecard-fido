//
//  BootloaderModeCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/01/11.
//

#ifndef BootloaderModeCommand_h
#define BootloaderModeCommand_h

#import <Foundation/Foundation.h>

@protocol BootloaderModeCommandDelegate;

@interface BootloaderModeCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestBootloaderMode;

@end

@protocol BootloaderModeCommandDelegate <NSObject>

    - (void)BootloaderModeDidCompleted:(bool)success message:(NSString *)message;

@end

#endif /* BootloaderModeCommand_h */
