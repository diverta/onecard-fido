//
//  MatterCommand.h
//  mattercontroller
//
//  Created by Makoto Morita on 2021/07/28.
//
#ifndef MatterCommand_h
#define MatterCommand_h

#import <CHIP/CHIP.h>

@interface MatterCommand : NSObject <CHIPDevicePairingDelegate>

    - (id)initWithViewControllerRef:(id)ref;

@end

#endif /* MatterCommand_h */
