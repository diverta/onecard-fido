//
//  ToolHIDHelper.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/07/03.
//
#ifndef ToolHIDHelper_h
#define ToolHIDHelper_h

@protocol ToolHIDHelperDelegate;

    @interface ToolHIDHelper : NSObject

    @property (nonatomic, weak) id<ToolHIDHelperDelegate> delegate;

    - (id)initWithDelegate:(id<ToolHIDHelperDelegate>)delegate;
    - (void)hidHelperWillSend:(NSData *)hidHelperMessage;
    - (void)hidHelperWillSendErrorResponse:(uint8_t)error_value;

@end

@protocol ToolHIDHelperDelegate <NSObject>

    - (void)hidHelperDidReceive:(NSData *)hidHelperMessages;

@end

#endif /* ToolHIDHelper_h */
