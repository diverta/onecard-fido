//
//  MatterCommand.h
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/28.
//
#ifndef MatterCommand_h
#define MatterCommand_h

#import <CHIP/CHIP.h>

@protocol MatterCommandDelegate;

@interface MatterCommand : NSObject <CHIPDevicePairingDelegate>

    - (id)initWithDelegate:(id<MatterCommandDelegate>)delegate;
    - (void)startBLEConnection;

@end

@protocol MatterCommandDelegate <NSObject>

    - (void)notifyMessage:(NSString *)message;
    - (void)didPairingComplete:(bool)success;

@end


#endif /* MatterCommand_h */
