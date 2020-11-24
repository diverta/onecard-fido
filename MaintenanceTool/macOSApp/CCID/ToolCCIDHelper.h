//
//  ToolCCIDHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/11/20.
//
#ifndef ToolCCIDHelper_h
#define ToolCCIDHelper_h

#import <CryptoTokenKit/CryptoTokenKit.h>

@interface ToolCCIDHelper : NSObject

    - (void)setSendParameters:(ToolCCIDCommand *)ref ins:(uint8_t)ins p1:(uint8_t)p1 p2:(uint8_t)p2 data:(NSData *)data le:(uint16_t)le;
    - (void)SCardSlotManagerWillBeginSession;

    // for weak reference
    - (void)SCardSlotManagerDidGetSlot:(TKSmartCardSlot *)slot withName:(NSString *)slotName;
    - (void)SCardSlotManagerDidBeginSession:(TKSmartCard *)card withReply:(bool)success error:(NSError *)error;

@end

#endif /* ToolCCIDHelper_h */
