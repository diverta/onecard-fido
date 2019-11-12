//
//  ToolBLEHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2018/01/22.
//
#ifndef ToolBLEHelper_h
#define ToolBLEHelper_h

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol ToolBLEHelperDelegate;

    @interface ToolBLEHelper : NSObject

    @property (nonatomic, strong) NSString *serviceName;
    @property (nonatomic, strong) NSArray  *serviceUUIDs;
    @property (nonatomic, strong) NSArray  *characteristicUUIDs;

    @property (nonatomic, weak)   id<ToolBLEHelperDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate;
    - (void)centralManagerWillConnect;
    - (void)centralManagerWillDisconnect;
    - (void)centralManagerWillSend:(NSArray<NSData *> *)bleMessages;
    - (void)centralManagerWillStartResponseTimeout;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;

    - (void)centralManagerDidConnect;
    - (void)centralManagerDidFailConnectionWith:(NSString *)message error:(NSError *)error;
    - (void)centralManagerDidReceive:(NSData *)bleMessage;
    - (void)centralManagerDidDisconnectWith:(NSString *)message error:(NSError *)error;

@end

#endif /* ToolBLEHelper_h */
