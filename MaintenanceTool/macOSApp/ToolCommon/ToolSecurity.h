//
//  ToolSecurity.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/03/30.
//
#ifndef ToolSecurity_h
#define ToolSecurity_h

#import <Foundation/Foundation.h>

@interface ToolSecurity : NSObject

    + (NSData *)generatePrivkeyDataFromPrivkeyBytes:(uint8_t *)privBytes withPubkeyBytes:(uint8_t *)pubBytes;
    + (id)generatePrivkeyFromData:(NSData *)privkeyData;
    + (id)generatePubkeyFromPrivkey:(id)privSecKeyRef;

    + (NSData *)createECDSASignatureWithData:(NSData *)data withPrivkeyRef:(id)privkey withAlgorithm:(SecKeyAlgorithm)algorithm;
    + (bool)verifyECDSASignature:(NSData *)signature withDataToSign:(NSData *)dataToSign withPubkeyRef:(id)pubkey withAlgorithm:(SecKeyAlgorithm)algorithm;

@end

#endif /* ToolSecurity_h */
