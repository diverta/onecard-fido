#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol ToolBLECentralDelegate;

    @interface ToolBLECentral : NSObject

    @property (nonatomic, strong) NSString *serviceName;
    @property (nonatomic, strong) NSArray  *serviceUUIDs;
    @property (nonatomic, strong) NSArray  *characteristicUUIDs;

    @property (nonatomic, weak)   id<ToolBLECentralDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECentralDelegate>)delegate;
    - (void)centralManagerWillConnect;
    - (void)centralManagerWillDisconnect;
    - (void)centralManagerWillSend:(NSArray<NSData *> *)bleMessages;
    - (void)centralManagerWillStartResponseTimeout;

@end

@protocol ToolBLECentralDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)notifyCentralManagerMessage:(NSString *)message;
    - (void)notifyCentralManagerErrorMessage:(NSString *)message error:(NSError *)error;

    - (void)centralManagerDidConnect;
    - (void)centralManagerDidFailConnection;
    - (void)centralManagerDidReceive:(NSData *)bleMessage;
    - (void)centralManagerDidDisconnect;

@end
