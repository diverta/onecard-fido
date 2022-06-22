//
//  AppHIDCommand.h
//  DevelopmentTool
//
//  Created by Makoto Morita on 2022/06/17.
//
#ifndef AppHIDCommand_h
#define AppHIDCommand_h

#import <Foundation/Foundation.h>
#import "AppDefine.h"

@protocol AppHIDCommandDelegate;

@interface AppHIDCommand : NSObject

    @property (nonatomic, weak) id delegate;

    - (id)initWithDelegate:(id)delegate;
    - (bool)checkUSBHIDConnection;
    - (void)doRequestCommand:(Command)command;

@end

@protocol AppHIDCommandDelegate <NSObject>

    - (void)didDetectConnect;
    - (void)didDetectRemoval;
    - (void)didResponseCommand:(Command)command response:(NSData *)response success:(bool)success errorMessage:(NSString *)errorMessage;

@end

#endif /* AppHIDCommand_h */
