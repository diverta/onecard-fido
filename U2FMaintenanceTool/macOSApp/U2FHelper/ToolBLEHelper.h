//
//  ToolBLEHelper.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/01/09.
//
#ifndef ToolBLEHelper_h
#define ToolBLEHelper_h

@protocol ToolBLEHelperDelegate;

    @interface ToolBLEHelper : NSObject

    @property (nonatomic, weak) id<ToolBLEHelperDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate;
    - (void)bleHelperWillSetStdinNotification;
    - (void)bleHelperWillSend:(NSDictionary *)bleHelperMessage;

    - (bool) bleHelperCommunicateAsChromeNative;
    - (bool) bleHelperHasSentMessageToChrome;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)bleHelperDidReceive:(NSArray<NSDictionary *> *)bleHelperMessages;
    - (void)bleHelperDidSend:(NSData *)chromeMessageData;

@end

#endif /* ToolBLEHelper_h */
