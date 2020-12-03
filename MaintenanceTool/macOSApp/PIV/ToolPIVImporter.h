//
//  ToolPIVImporter.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVImporter_h
#define ToolPIVImporter_h

@interface ToolPIVImporter : NSObject

    - (id)initForKeySlot:(uint8_t)keySlotId;

    - (bool)readPrivateKeyPemFrom:(NSString *)pemFilePath;
    - (bool)readCertificatePemFrom:(NSString *)pemFilePath;

    - (NSData *)getPrivateKeyAPDUData;
    - (NSData *)getCertificateAPDUData;

@end

#endif /* ToolPIVImporter_h */
