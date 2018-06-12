#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol ToolBLECentralDelegate;

    @interface ToolBLECentral : NSObject

    @property (nonatomic, strong) NSString *serviceName;
    @property (nonatomic, strong) NSArray  *serviceUUIDs;

    @property (nonatomic, weak)   id<ToolBLECentralDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECentralDelegate>)delegate;
    - (void)centralManagerWillConnect;
    - (void)centralManagerWillDisconnect;
    - (void)centralManagerWillStartSubscribe;
    - (void)centralManagerWillSend:(NSArray<NSData *> *)bleMessages;
    - (void)centralManagerWillStartResponseTimeout;
    - (void)centralManagerWillReadParingModeSign;
    - (bool)centralManagerHasParingModeSign;

@end

@protocol ToolBLECentralDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)notifyCentralManagerMessage:(NSString *)message;

    - (void)centralManagerDidStartSubscribe;
    - (void)centralManagerDidConnect;
    - (void)centralManagerDidFailConnectionWith:(NSString *)message error:(NSError *)error;
    - (void)centralManagerDidReceive:(NSData *)bleMessage;
    - (void)centralManagerDidDisconnectWith:(NSString *)message error:(NSError *)error;

@end
