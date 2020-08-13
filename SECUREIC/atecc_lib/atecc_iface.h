/* 
 * File:   atecc_iface.h
 * Author: makmorit
 *
 * Created on 2020/08/11, 12:22
 */
#ifndef ATECC_IFACE_H
#define ATECC_IFACE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atecc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ATECC_I2C_IFACE,
} ATECC_IFACE_TYPE;

typedef enum {
    ATECC508A,
    ATECC608A,
} ATECC_DEVICE_TYPE;

typedef struct {
    ATECC_IFACE_TYPE  iface_type;   // active iface - how to interpret the union below
    ATECC_DEVICE_TYPE devtype;      // explicit device type
    struct {
        uint8_t  slave_address;     // 8-bit slave address
        uint8_t  bus;               // logical i2c bus number, 0-based - HAL will map this to a pin pair for SDA SCL
        uint32_t baud;              // typically 400000
    } i2c;
    uint16_t wake_delay;            // microseconds of tWHI + tWLO which varies based on chip type
    int      rx_retries;            // the number of retries to attempt for receiving bytes
    void *   cfg_data;              // opaque data used by HAL in device discovery
} ATECC_IFACE_CFG;

typedef struct atecc_iface *ATECC_IFACE;

struct atecc_iface {
    ATECC_IFACE_TYPE mType;
    ATECC_IFACE_CFG *mIfaceCFG;

    ATECC_STATUS (*init_func)(ATECC_IFACE hal);
    ATECC_STATUS (*postinit_func)(ATECC_IFACE hal);
    ATECC_STATUS (*send_func)(ATECC_IFACE hal, uint8_t *txdata, int txlength);
    ATECC_STATUS (*receive_func)(ATECC_IFACE hal, uint8_t *rxdata, uint16_t *rxlength);
    ATECC_STATUS (*wake_func)(ATECC_IFACE hal);
    ATECC_STATUS (*idle_func)(ATECC_IFACE hal);
    ATECC_STATUS (*sleep_func)(ATECC_IFACE hal);

    void *hal_data;
};

//
// 関数群
//
ATECC_STATUS atecc_iface_init(ATECC_IFACE_CFG *cfg, ATECC_IFACE iface);
ATECC_STATUS atecc_iface_release(ATECC_IFACE iface);

ATECC_STATUS atecc_iface_init_func(ATECC_IFACE iface);
ATECC_STATUS atecc_iface_postinit_func(ATECC_IFACE iface);
ATECC_STATUS atecc_iface_send_func(ATECC_IFACE iface, uint8_t *txdata, int txlength);
ATECC_STATUS atecc_iface_receive_func(ATECC_IFACE iface, uint8_t *rxdata, uint16_t *rxlength);
ATECC_STATUS atecc_iface_wake_func(ATECC_IFACE iface);
ATECC_STATUS atecc_iface_idle_func(ATECC_IFACE iface);
ATECC_STATUS atecc_iface_sleep_func(ATECC_IFACE iface);

//
// ハードウェア依存の関数
// 実装は atecc608a_i2c_hal.c にあります。
//
ATECC_STATUS hal_iface_init(ATECC_IFACE iface);
ATECC_STATUS hal_iface_release(void *hal_data);
void         atecc_delay_us(uint32_t delay);
void         atecc_delay_10us(uint32_t delay);
void         atecc_delay_ms(uint32_t delay);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_IFACE_H */
