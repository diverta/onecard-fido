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

@end

@protocol AppHIDCommandDelegate <NSObject>

    - (void)didDetectConnect;
    - (void)didDetectRemoval;

@end

#endif /* AppHIDCommand_h */
