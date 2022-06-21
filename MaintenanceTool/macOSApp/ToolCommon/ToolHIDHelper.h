//
//  ToolHIDHelper.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/06/17.
//
#ifndef ToolHIDHelper_h
#define ToolHIDHelper_h

#import <Foundation/Foundation.h>

@protocol ToolHIDHelperDelegate;

@interface ToolHIDHelper : NSObject

@property (nonatomic, weak) id<ToolHIDHelperDelegate> delegate;

    - (id)initWithDelegate:(id<ToolHIDHelperDelegate>)delegate;
    - (bool)isDeviceConnected;
    - (void)hidHelperWillSend:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;

@end

@protocol ToolHIDHelperDelegate <NSObject>

    - (void)hidHelperDidReceive:(NSData *)message CID:(NSData *)cid CMD:(uint8_t)cmd;
    - (void)hidHelperDidResponseTimeout;

    - (void)hidHelperDidDetectConnect;
    - (void)hidHelperDidDetectRemoval;

@end

#endif /* ToolHIDHelper_h */
