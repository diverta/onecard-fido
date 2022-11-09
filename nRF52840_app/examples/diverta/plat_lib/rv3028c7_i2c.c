/* 
 * File:   rv3028c7_i2c.c
 * Author: makmorit
 *
 * Created on 2022/11/09, 15:43
 */
#include <stdbool.h>

// for logging informations
#define NRF_LOG_MODULE_NAME rv3028c7_i2c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for I2C access
#include "fido_twi.h"

// I2Cスレーブアドレス
#define RV3028C7_ADDRESS                0x52

// レジスターアドレス
#define RV3028C7_REG_CONTROL_2          0x10

// データ送受信用の一時領域
static uint8_t read_buff[32];
static uint8_t write_buff[32];

//
// I2C write & read
//
static bool read_register(uint8_t reg_addr, uint8_t *reg_val)
{
    write_buff[0] = reg_addr;

    // Send the address to read from
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 1) == false) {
        return false;
    }

    // Read from device. STOP after this
    if (fido_twi_read(RV3028C7_ADDRESS, read_buff, 1) == false) {
        return false;
    }

    *reg_val = read_buff[0];
    return true;
}

//
// RTCCの初期化
//
bool rv3028c7_initialize(void)
{
    // デバイスの初期化
    if (fido_twi_init() == false) {
        NRF_LOG_DEBUG("rv3028c7_initialize failed: Communication with device failed. Same as in hardware dependent modules.");
        return false;
    }

    // 制御レジスター（Control 2 register）を参照、0x00なら正常
    uint8_t c2;
    if (read_register(RV3028C7_REG_CONTROL_2, &c2) == false) {
        return false;
    }
    if (c2 != 0x00) {
        NRF_LOG_DEBUG("RTCC is not available");
        return false;
    }
    
    return true;
}