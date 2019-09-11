//
//  ToolHIDHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/03/19.
//

#ifndef ToolHIDHelper_h
#define ToolHIDHelper_h

@protocol ToolHIDHelperDelegate;

@interface ToolHIDHelper : NSObject

@property (nonatomic, weak) id<ToolHIDHelperDelegate> delegate;

    - (id)initWithDelegate:(id<ToolHIDHelperDelegate>)delegate;
    - (bool)isDeviceConnected;
    - (void)hidHelperWillSend:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;

@end

@protocol ToolHIDHelperDelegate <NSObject>

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;

@end

#endif /* ToolHIDHelper_h */
