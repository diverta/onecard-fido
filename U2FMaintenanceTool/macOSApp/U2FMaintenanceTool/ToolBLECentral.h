#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>

@protocol ToolBLECentralDelegate;

    @interface ToolBLECentral : NSObject

    @property (nonatomic, strong) NSString *serviceName;
    @property (nonatomic, strong) NSArray  *serviceUUIDs;
    @property (nonatomic, strong) NSArray  *characteristicUUIDs;

    @property (nonatomic, weak)   id<ToolBLECentralDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECentralDelegate>)delegate;
    - (void)doConnect;
    - (void)disconnect;
    - (void)sendBleMessages:(NSArray<NSData *> *)bleMessages;

@end

@protocol ToolBLECentralDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)notifyCentralManagerConnected;
    - (void)notifyCentralManagerConnectFailed:(NSString *)message;
    - (void)notifyCentralManagerMessage:(NSString *)message;

    - (void)bleMessageDidReceive:(NSData *)bleMessage;
@end
