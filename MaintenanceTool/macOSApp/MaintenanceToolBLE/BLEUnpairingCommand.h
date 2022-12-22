//
//  BLEUnpairingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/09.
//
#ifndef BLEUnpairingCommand_h
#define BLEUnpairingCommand_h

#import <Foundation/Foundation.h>

@protocol BLEUnpairingCommandDelegate;

@interface BLEUnpairingCommand : NSObject

    - (id)initWithDelegate:(id)delegate withUnpairingRequestWindowRef:(id)windowRef;
    - (void)invokeUnpairingRequestProcess;

@end

@protocol BLEUnpairingCommandDelegate <NSObject>

    - (void)doResponseBLESettingCommand:(bool)success message:(NSString *)message;
    - (void)notifyCommandMessageToMainUI:(NSString *)message;

@end

#endif /* BLEUnpairingCommand_h */
