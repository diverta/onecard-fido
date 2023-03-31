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
    + (NSData *)generatePubkeyDataFromPubkeyBytes:(uint8_t *)pubBytes;
    + (id)generatePrivkeyFromData:(NSData *)privkeyData;
    + (id)generatePubkeyFromData:(NSData *)pubkeyData;
    + (id)generatePublicSecKeyRefFromPubkeyBytes:(uint8_t *)pubBytes;
    + (id)generatePrivkeyFromRandom;
    + (id)generatePubkeyFromPrivkey:(id)privSecKeyRef;
    + (bool)getKeyFromPrivateSecKeyRef:(id)secKeyRef toPrivkeyBuffer:(uint8_t *)privkeyBytes toPubkeyBuffer:(uint8_t *)pubkeyBytes;

    + (NSData *)createECDSASignatureWithData:(NSData *)data withPrivkeyRef:(id)privkey withAlgorithm:(SecKeyAlgorithm)algorithm;
    + (bool)verifyECDSASignature:(NSData *)signature withDataToSign:(NSData *)dataToSign withPubkeyRef:(id)pubkey withAlgorithm:(SecKeyAlgorithm)algorithm;

@end

#endif /* ToolSecurity_h */
