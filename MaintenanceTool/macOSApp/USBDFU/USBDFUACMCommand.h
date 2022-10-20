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

@end

@protocol USBDFUACMCommandDelegate <NSObject>

    - (void)didEstablishACMConnection:(bool)success;

@end

#endif /* USBDFUACMCommand_h */
