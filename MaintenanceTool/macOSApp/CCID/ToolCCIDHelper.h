//
//  ToolCCIDHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#ifndef ToolCCIDHelper_h
#define ToolCCIDHelper_h

#import <CryptoTokenKit/CryptoTokenKit.h>
#import "ToolCommon.h"

@protocol ToolCCIDHelperDelegate;

@interface ToolCCIDHelper : NSObject

    - (id)initWithDelegate:(id<ToolCCIDHelperDelegate>)delegate;
    - (void)SCardSlotManagerWillBeginSession:(id)ref ins:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le;

    // for weak reference
    - (void)SCardSlotManagerDidGetSlot:(TKSmartCardSlot *)slot withName:(NSString *)slotName;
    - (void)SCardSlotManagerDidBeginSession:(TKSmartCard *)card withReply:(bool)success error:(NSError *)error;

@end

@protocol ToolCCIDHelperDelegate <NSObject>

    - (void)ccidHelperDidProcess:(bool)success response:(NSData *)resp status:(uint16_t)sw;

@end

#endif /* ToolCCIDHelper_h */
