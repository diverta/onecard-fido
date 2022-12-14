//
//  BLEPairingCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/07/19.
//
#ifndef BLEPairingCommand_h
#define BLEPairingCommand_h

#import <Foundation/Foundation.h>

@protocol BLEPairingCommandDelegate;

@interface BLEPairingCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)doRequestBLEPairing;

@end

@protocol BLEPairingCommandDelegate <NSObject>

    - (void)doResponseBLESettingCommand:(bool)success message:(NSString *)message;
    - (void)notifyCommandMessageToMainUI:(NSString *)message;

@end

#endif /* BLEPairingCommand_h */
