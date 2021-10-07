//
//  ToolBLEConnectionHelper.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2021/10/07.
//
#ifndef ToolBLEConnectionHelper_h
#define ToolBLEConnectionHelper_h

#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol ToolBLEConnectionHelperDelegate;

@interface ToolBLEConnectionHelper : NSObject

    - (id)initWithDelegate:(id<ToolBLEConnectionHelperDelegate>)delegate;
    - (void)helperWillConnectWithUUID:(NSString *)uuidString;
    - (void)helperWillDisconnect;
    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString;
    - (void)helperWillDiscoverCharacteristicsWithUUIDs:(NSArray<NSString *> *)uuids;

@end

@protocol ToolBLEConnectionHelperDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)helperDidConnectPeripheral;
    - (void)helperDidFailConnectionWith:(NSString *)message error:(NSError *)error;
    - (void)helperDidDisconnectWith:(NSString *)message error:(NSError *)error;
    - (void)helperDidDiscoverService;
    - (void)helperDidDiscoverCharacteristics;

@end

#endif /* ToolBLEConnectionHelper_h */
