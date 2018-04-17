//
//  ToolTimer.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/04/17.
//
#ifndef ToolTimer_h
#define ToolTimer_h

@protocol ToolTimerDelegate;

@interface ToolTimer : NSObject

    @property (nonatomic, weak) id<ToolTimerDelegate> delegate;

    - (id)initWithDelegate:(id<ToolTimerDelegate>)delegate;

    - (void)startScanningTimeoutMonitor;
    - (void)cancelScanningTimeoutMonitor;

    - (void)startConnectionTimeoutMonitor:(CBPeripheral *)peripheral;
    - (void)cancelConnectionTimeoutMonitor:(CBPeripheral *)peripheral;

@end

@protocol ToolTimerDelegate <NSObject>

    - (void)scanningDidTimeout;
    - (void)connectionDidTimeout;

@end

#endif /* ToolTimer_h */
