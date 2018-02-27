//
//  ToolParameters.h
//  U2FMaintenanceTool
//
//  Created by Makoto Morita on 2018/02/27.
//
#ifndef ToolParameters_h
#define ToolParameters_h

@interface SelfCertParameter : NSObject

    @property (nonatomic) NSString *csrPath;
    @property (nonatomic) NSString *outPath;

@end

#endif /* ToolParameters_h */
