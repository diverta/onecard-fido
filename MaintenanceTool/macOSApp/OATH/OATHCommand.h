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
    @property (nonatomic) TransportType transportType;
    @property (nonatomic) NSString     *resultInformativeMessage;
    @property (nonatomic) NSString     *oathAccountName;
    @property (nonatomic) NSString     *oathAccountIssuer;
    @property (nonatomic) NSString     *oathBase32Secret;

@end

@interface OATHCommand : NSObject

    @property (nonatomic) OATHCommandParameter     *parameter;

    + (OATHCommand *)instance;
    - (bool)isUSBCCIDCanConnect;
    - (bool)scanQRCode;

@end

#endif /* OATHCommand_h */
