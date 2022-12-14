//
//  EraseBondsCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/12/14.
//
#ifndef EraseBondsCommand_h
#define EraseBondsCommand_h

#import <Foundation/Foundation.h>

@protocol EraseBondsCommandDelegate;

@interface EraseBondsCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (bool)isUSBHIDConnected;
    - (void)doRequestHidEraseBonds;

@end

@protocol EraseBondsCommandDelegate <NSObject>

    - (void)doResponseBLESettingCommand:(bool)success message:(NSString *)message;
    - (void)notifyCommandMessageToMainUI:(NSString *)message;

@end

#endif /* EraseBondsCommand_h */
