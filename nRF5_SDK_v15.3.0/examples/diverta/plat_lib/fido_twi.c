/* 
 * File:   fido_twi.c
 * Author: makmorit
 *
 * Created on 2019/11/20, 11:10
 */
#include <sdk_config.h>

#include "sdk_common.h"
#include "boards.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_twi
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for pin assign
#include "fido_board.h"

// for debug data
#define LOG_DEBUG_HEX_DATA false

// 初期化処理の重複実行抑止フラグ
static bool twi_init_done = false;

// TWI instance
#define TWI_INSTANCE_ID 0
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

void const *fido_twi_instance_ref(void)
{
    return (void const *)&m_twi;
}

void fido_twi_init (void)
{
    // 初期化処理の重複実行抑止
    if (twi_init_done) {
        return;
    } else {
        twi_init_done = true;
    }

    const nrf_drv_twi_config_t twi_config = {
       .scl                = TWI_SCL_PIN,
       .sda                = TWI_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    ret_code_t err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);
    nrf_drv_twi_enable(&m_twi);
}

bool fido_twi_write(uint8_t address, uint8_t *p_data, uint8_t length)
{
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, address, p_data, length, false);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("ATECC608A hal_i2c_send: nrf_drv_twi_tx returns %d ", err_code);
        return false;
    }
    return true;
}

bool fido_twi_read(uint8_t address, uint8_t *p_data, uint8_t length)
{
    ret_code_t err_code = nrf_drv_twi_rx(&m_twi, address, p_data, length);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("ATECC608A hal_i2c_receive: nrf_drv_twi_rx returns %d ", err_code);
        return false;
    }
#if LOG_DEBUG_HEX_DATA
    NRF_LOG_DEBUG("Read from ATECC608A (%d bytes):", length);
    NRF_LOG_HEXDUMP_DEBUG(p_data, length);
#endif
    return true;
}

bool fido_twi_verify_nack(uint8_t address)
{
    static uint8_t tx[1] = {0};
    ret_code_t err_code = nrf_drv_twi_tx(&m_twi, address, tx, 1, false);
    if (err_code == NRF_ERROR_DRV_TWI_ERR_ANACK) {
        return true;

    } else if (err_code == NRF_ERROR_DRV_TWI_ERR_DNACK) {
        return true;

    } else if (err_code == NRF_SUCCESS) {
        return true;

    } else {
        NRF_LOG_ERROR("verify_nack: nrf_drv_twi_tx returns %d ", err_code);
        return false;
    }
}
