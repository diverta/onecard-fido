//
//  ToolLogFile.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/11/11.
//
#ifndef ToolLogFile_h
#define ToolLogFile_h

// ログ種別
typedef enum : NSInteger {
    LOG_TYPE_NONE = 0,
    LOG_TYPE_ERROR,
    LOG_TYPE_WARN,
    LOG_TYPE_INFO,
    LOG_TYPE_DEBUG
} LogType;

@interface ToolLogFile : NSObject

    + (ToolLogFile *)defaultLogger;

    - (void)error:(NSString *)logMessage;
    - (void)errorWithFormat:(NSString *)format, ...;
    - (void)warn:(NSString *)logMessage;
    - (void)warnWithFormat:(NSString *)format, ...;
    - (void)info:(NSString *)logMessage;
    - (void)infoWithFormat:(NSString *)format, ...;
    - (void)debug:(NSString *)logMessage;
    - (void)debugWithFormat:(NSString *)format, ...;

@end

#endif /* ToolLogFile_h */
