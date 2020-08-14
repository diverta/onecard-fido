/* 
 * File:   atecc_device.h
 * Author: makmorit
 *
 * Created on 2020/08/11, 10:19
 */
#ifndef ATECC_DEVICE_H
#define ATECC_DEVICE_H

#include "atecc_iface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ATECC_ZONE_CONFIG                ((uint8_t)0x00)
#define ATECC_ZONE_OTP                   ((uint8_t)0x01)
#define ATECC_ZONE_DATA                  ((uint8_t)0x02)
#define ATECC_ZONE_MASK                  ((uint8_t)0x03)
#define ATECC_ZONE_ENCRYPTED             ((uint8_t)0x40)
#define ATECC_ZONE_READWRITE_32          ((uint8_t)0x80)
#define ATECC_ADDRESS_MASK_CONFIG        (0x001F)
#define ATECC_ADDRESS_MASK_OTP           (0x000F)
#define ATECC_ADDRESS_MASK               (0x007F)
#define ATECC_TEMPKEY_KEYID              (0xFFFF)

#define ATECC_CHIPMODE_OFFSET           (19)
#define ATECC_CHIPMODE_I2C_ADDRESS_FLAG ((uint8_t)0x01)
#define ATECC_CHIPMODE_TTL_ENABLE_FLAG  ((uint8_t)0x02)
#define ATECC_CHIPMODE_WATCHDOG_MASK    ((uint8_t)0x04)
#define ATECC_CHIPMODE_WATCHDOG_SHORT   ((uint8_t)0x00)
#define ATECC_CHIPMODE_WATCHDOG_LONG    ((uint8_t)0x04)
#define ATECC_CHIPMODE_CLOCK_DIV_MASK   ((uint8_t)0xF8)
#define ATECC_CHIPMODE_CLOCK_DIV_M0     ((uint8_t)0x00)
#define ATECC_CHIPMODE_CLOCK_DIV_M1     ((uint8_t)0x28)
#define ATECC_CHIPMODE_CLOCK_DIV_M2     ((uint8_t)0x68)

typedef struct atecc_command *ATECC_COMMAND;
typedef struct atecc_device  *ATECC_DEVICE;

struct atecc_command {
    ATECC_DEVICE_TYPE dt;
    uint8_t           clock_divider;
    uint16_t          execution_time_msec;
};

struct atecc_device {
    ATECC_COMMAND mCommands;
    ATECC_IFACE   mIface;
};

//
// 関数群
//
ATECC_DEVICE atecc_device_ref(void);
bool         atecc_device_init(void);
bool         atecc_device_release(void);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_DEVICE_H */
