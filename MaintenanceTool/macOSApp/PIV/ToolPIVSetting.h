//
//  ToolPIVSetting.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/12.
//
#ifndef ToolPIVSetting_h
#define ToolPIVSetting_h

@interface ToolPIVSetting : NSObject

    - (id)initWithSlotName:(NSString *)name;
    - (void)setRetryCount:(uint8_t)retries;
    - (void)setDataObject:(NSData *)object forObjectId:(unsigned int)objectId;

@end

#endif /* ToolPIVSetting_h */
