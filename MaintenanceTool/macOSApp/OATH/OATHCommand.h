//
//  OATHCommand.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/13.
//

#ifndef OATHCommand_h
#define OATHCommand_h

#import "AppDefine.h"

@interface OATHCommandParameter : NSObject

    @property (nonatomic) Command       command;
    @property (nonatomic) NSString     *commandTitle;
    @property (nonatomic) bool          commandSuccess;
    @property (nonatomic) TransportType transportType;
    @property (nonatomic) NSString     *resultMessage;
    @property (nonatomic) NSString     *resultInformativeMessage;
    @property (nonatomic) NSString     *oathAccountName;
    @property (nonatomic) NSString     *oathAccountIssuer;
    @property (nonatomic) NSString     *oathBase32Secret;
    @property (nonatomic) uint32_t      oathTotpValue;

    - (NSString *)oathAccount;

@end

@interface OATHCommand : NSObject

    @property (nonatomic) OATHCommandParameter     *parameter;

    + (OATHCommand *)instance;
    - (bool)isUSBCCIDCanConnect;
    - (bool)scanQRCode;
    - (void)commandWillPerformForTarget:(id)object forSelector:(SEL)selector;

@end

#endif /* OATHCommand_h */
