//
//  ToolPIVImporter.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/01.
//
#ifndef ToolPIVImporter_h
#define ToolPIVImporter_h

#import <Foundation/Foundation.h>

@interface ToolPIVImporter : NSObject

    - (id)initForKeySlot:(uint8_t)keySlotId;

    - (bool)readPrivateKeyPemFrom:(NSString *)pemFilePath;
    - (bool)readCertificatePemFrom:(NSString *)pemFilePath;

    - (NSData *)getPrivateKeyAPDUData;
    - (NSData *)getCertificateAPDUData;

    // 処理対象となるスロットID、アルゴリズムを保持
    @property (nonatomic) uint8_t           keySlotId;
    @property (nonatomic) uint8_t           keyAlgorithm;
    @property (nonatomic) uint8_t           certAlgorithm;

@end

#endif /* ToolPIVImporter_h */
