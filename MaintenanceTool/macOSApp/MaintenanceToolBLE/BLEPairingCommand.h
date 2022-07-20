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
    - (bool)isUSBHIDConnected;
    - (void)doRequestHidEraseBonds;
    - (void)doRequestBlePairing;

@end

@protocol BLEPairingCommandDelegate <NSObject>

    - (void)notifyMessage:(NSString *)message;
    - (void)doResponseBLEPairing:(bool)success message:(NSString *)message;

@end

#endif /* BLEPairingCommand_h */
