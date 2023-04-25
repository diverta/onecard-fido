//
//  ToolBLEHelper.h
//  ToolCommon
//
//  Created by Makoto Morita on 2022/07/12.
//
#ifndef ToolBLEHelper_h
#define ToolBLEHelper_h

#import <Foundation/Foundation.h>

@protocol ToolBLEHelperDelegate;

@interface ToolBLEHelper : NSObject

    - (id)initWithDelegate:(id<ToolBLEHelperDelegate>)delegate;
    - (void)clearDiscoveredPeripheral;
    - (void)helperWillConnectWithUUID:(NSString *)uuidString;
    - (void)helperWillDisconnect;
    - (void)helperWillDisconnectForce:(id)peripheralRef;
    - (void)helperWillConnectPeripheral:(id)peripheralRef;
    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString;
    - (void)helperWillDiscoverCharacteristics:(id)serviceRef withUUIDs:(NSArray<NSString *> *)uuids;
    - (void)helperWillSubscribeCharacteristic:(id)serviceRef;
    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage;
    - (void)helperWillReadForCharacteristics;
    - (bool)helperIsSubscribingCharacteristic;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)helperDidScanForPeripheral:(id)peripheralRef scannedPeripheralName:(NSString *)peripheralName withUUID:(NSString *)uuidString withServiceData:(NSData *)serviceData;
    - (void)helperDidConnectPeripheral;
    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(NSUInteger)reason;
    - (void)helperDidDisconnectWithError:(NSError *)error peripheral:(id)peripheralRef;
    - (void)helperDidDiscoverService:(id)serviceRef;
    - (void)helperDidDiscoverCharacteristics:(id)serviceRef;
    - (void)helperDidSubscribeCharacteristic;
    - (void)helperDidWriteForCharacteristics;
    - (void)helperDidReadForCharacteristic:(NSData *)responseMessage;

@end

#endif /* ToolBLEHelper_h */
