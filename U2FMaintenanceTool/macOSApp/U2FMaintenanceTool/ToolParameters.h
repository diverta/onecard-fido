//
//  ToolParameters.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#ifndef ToolParameters_h
#define ToolParameters_h


@interface KeyPairParameter : NSObject

    @property (nonatomic) NSString *outPath;

@end

@interface CertReqParameter : NSObject

    @property (nonatomic) NSString *pemPath;
    @property (nonatomic) NSString *CN;
    @property (nonatomic) NSString *OU;
    @property (nonatomic) NSString *O;
    @property (nonatomic) NSString *L;
    @property (nonatomic) NSString *ST;
    @property (nonatomic) NSString *C;
    @property (nonatomic) NSString *outPath;

@end

@interface SelfCertParameter : NSObject

    @property (nonatomic) NSString *csrPath;
    @property (nonatomic) NSString *outPath;

@end

#endif /* ToolParameters_h */
