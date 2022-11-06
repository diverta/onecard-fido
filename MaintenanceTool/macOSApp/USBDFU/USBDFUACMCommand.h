//
//  USBDFUACMCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2022/10/20.
//
#ifndef USBDFUACMCommand_h
#define USBDFUACMCommand_h

@protocol USBDFUACMCommandDelegate;

@interface USBDFUACMCommand : NSObject

    - (id)initWithDelegate:(id)delegate;
    - (void)establishACMConnection;
    - (void)closeACMConnection;
    - (NSData *)sendRequest:(NSData *)data timeoutSec:(double)timeout;
    - (bool)sendRequestData:(NSData *)data;
    - (bool)assertDFUResponseSuccess:(NSData *)response;

@end

@protocol USBDFUACMCommandDelegate <NSObject>

    - (void)didEstablishACMConnection:(bool)success;

@end

#endif /* USBDFUACMCommand_h */
