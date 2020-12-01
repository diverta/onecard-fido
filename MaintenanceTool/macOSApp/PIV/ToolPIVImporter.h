//
//  ToolPIVImporter.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVImporter_h
#define ToolPIVImporter_h

@interface ToolPIVImporter : NSObject

    - (bool)readPrivateKeyPemFrom:(NSString *)pemFilePath;
    - (bool)readCertificatePemFrom:(NSString *)pemFilePath;

@end

#endif /* ToolPIVImporter_h */
