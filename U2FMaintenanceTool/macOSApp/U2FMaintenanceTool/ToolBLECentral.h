#import <Foundation/Foundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import "ToolCommand.h"

@protocol ToolBLECentralDelegate;

    @interface ToolBLECentral : NSObject

    @property (nonatomic, strong) NSString *serviceName;
    @property (nonatomic, strong) NSArray  *serviceUUIDs;
    @property (nonatomic, strong) NSArray  *characteristicUUIDs;

    @property (nonatomic, weak)   id<ToolBLECentralDelegate> delegate;

    - (id)initWithDelegate:(id<ToolBLECentralDelegate>)delegate;
    - (void)doCommand:(ToolCommand *)toolCommand;
    - (void)disconnect;

@end

@protocol ToolBLECentralDelegate <NSObject>

    - (void)notifyCentralManagerStateUpdate:(CBCentralManagerState)state;
    - (void)notifyFailWithMessage:(NSString *)errorMessage;
    - (void)notifySuccess;
    - (void)notifyMessage:(NSString *)message;

@end
