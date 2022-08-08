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
    - (void)helperWillConnectWithUUID:(NSString *)uuidString;
    - (void)helperWillDisconnect;
    - (void)helperWillConnectPeripheral:(id)peripheralRef;
    - (void)helperWillDiscoverServiceWithUUID:(NSString *)uuidString;
    - (void)helperWillDiscoverCharacteristicsWithUUIDs:(NSArray<NSString *> *)uuids;
    - (void)helperWillSubscribeCharacteristicWithTimeout:(NSTimeInterval)timeoutSec;
    - (void)helperWillWriteForCharacteristics:(NSData *)requestMessage;
    - (void)helperWillReadForCharacteristics;
    - (bool)helperIsSubscribingCharacteristic;

@end

@protocol ToolBLEHelperDelegate <NSObject>

    - (void)helperDidScanForPeripheral:(id)peripheralRef withUUID:(NSString *)uuidString;
    - (void)helperDidConnectPeripheral;
    - (void)helperDidFailConnectionWithError:(NSError *)error reason:(NSUInteger)reason;
    - (void)helperDidDisconnectWithError:(NSError *)error;
    - (void)helperDidDiscoverService;
    - (void)helperDidDiscoverCharacteristics;
    - (void)helperDidSubscribeCharacteristic;
    - (void)helperDidWriteForCharacteristics;
    - (void)helperDidReadForCharacteristic:(NSData *)responseMessage;

@end

#endif /* ToolBLEHelper_h */
