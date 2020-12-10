//
//  ToolCCIDHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#ifndef ToolCCIDHelper_h
#define ToolCCIDHelper_h

#import "ToolCommon.h"

@protocol ToolCCIDHelperDelegate;

@interface ToolCCIDHelper : NSObject

    - (id)initWithDelegate:(id<ToolCCIDHelperDelegate>)delegate;
    - (bool)ccidHelperWillConnect;
    - (void)ccidHelperWillDisconnect;
    - (void)ccidHelperWillSendIns:(uint8_t)sendIns p1:(uint8_t)sendP1 p2:(uint8_t)sendP2 data:(NSData *)sendData le:(uint16_t)sendLe;

@end

@protocol ToolCCIDHelperDelegate <NSObject>

    - (void)ccidHelperDidReceiveResponse:(NSData *)resp status:(uint16_t)sw;

@end

#endif /* ToolCCIDHelper_h */
