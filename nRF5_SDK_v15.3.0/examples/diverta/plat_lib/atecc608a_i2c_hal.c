/* 
 * File:   atecc608a_i2c_hal.c
 * Author: makmorit
 *
 * Created on 2019/11/25, 12:45
 */
#include "sdk_common.h"
#include "nrf_drv_twi.h"
#include "nrf_delay.h"

// for logging informations
#define NRF_LOG_MODULE_NAME atecc608a_i2c_hal
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for I2C access
#include "fido_twi.h"

//
// ATECC608A HAL関連
//
#include "hal/atca_hal.h"

// HAL structure, use this to store data
typedef struct {
    bool     active;
    uint8_t  slave_address;
    uint8_t  bus;
    uint32_t baud;
    uint16_t wake_delay;
    int      rx_retries;
    nrf_drv_twi_t const *p_instance;
} i2c_hal_data_t;

static i2c_hal_data_t m_hal_data;

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg) 
{
    fido_twi_init();

    m_hal_data.active = true;
    m_hal_data.slave_address = cfg->atcai2c.slave_address;
    m_hal_data.bus = cfg->atcai2c.bus;
    m_hal_data.baud = cfg->atcai2c.baud;
    m_hal_data.wake_delay = cfg->wake_delay;
    m_hal_data.rx_retries = cfg->rx_retries;
    m_hal_data.p_instance = (nrf_drv_twi_t const *)fido_twi_instance_ref();
    ((ATCAHAL_t*)hal)->hal_data = &m_hal_data;

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_post_init(ATCAIface iface) {
    UNUSED_PARAMETER(iface);
    return ATCA_SUCCESS;
}

static uint8_t get_twi_address(ATCAIface iface)
{
    // 7bitアドレスを取得
    i2c_hal_data_t *hal_data = (i2c_hal_data_t *)(iface->hal_data);
    uint8_t address = (hal_data->slave_address >> 1) & 0x7f;
    return address;
}

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *data, int length) 
{
    // 先頭バイトを差替えて送信
    data[0] = 0x3;
    length++;
    if (fido_twi_write(get_twi_address(iface), data, length) == false) {
        NRF_LOG_ERROR("hal_i2c_send failed");
        return ATCA_TX_FAIL;
    }

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rx_data, uint16_t *rx_length) 
{
    // read procedure is:
    // 1. read 1 byte, this will be the length of the package
    // 2. read the rest of the package
    i2c_hal_data_t *hal_data = (i2c_hal_data_t *)(iface->hal_data);
    uint8_t length_package[1] = {0};
    bool r = false;
    int retries = hal_data->rx_retries;
    while (--retries > 0 && r == false) {
        r = fido_twi_read(get_twi_address(iface), length_package, sizeof(length_package));
    }
    if (r == false) {
        NRF_LOG_ERROR("hal_i2c_receive failed: length read timeout");
        return ATCA_RX_TIMEOUT;
    }

    // データの１バイト目に、受信できるバイト数が格納されています
    uint8_t bytes_to_read = length_package[0] - 1;
    if (bytes_to_read > *rx_length) {
        NRF_LOG_ERROR("hal_i2c_receive buffer too small, requested %u, but have %u", bytes_to_read, *rx_length);
        return ATCA_SMALL_BUFFER;
    }

    // データ長を格納
    memset(rx_data, 0, *rx_length);
    rx_data[0] = length_package[0];

    // 残りのデータを受信
    r = false;
    retries = hal_data->rx_retries;
    while (--retries > 0 && r == false) {
        r = fido_twi_read(get_twi_address(iface), rx_data + 1, bytes_to_read);
    }
    if (r == false) {
        NRF_LOG_ERROR("hal_i2c_receive failed: package read timeout");
        return ATCA_RX_TIMEOUT;
    }
    *rx_length = length_package[0];

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_wake(ATCAIface iface) 
{
    // steps to wake the chip up...
    // 1. switch to 100KHz
    // 2. Send NULL buffer to address 0x0 (NACK)
    fido_twi_verify_nack(get_twi_address(iface));

    // 3. Wait for wake_delay
    i2c_hal_data_t *hal_data = (i2c_hal_data_t*)(iface->hal_data);
    nrf_delay_us(hal_data->wake_delay);

    // 4. Read from normal slave_address
    uint8_t rx_buffer[4] = {0};
    bool r = false;
    int retries = hal_data->rx_retries;
    while (--retries > 0 && r == false) {
        r = fido_twi_read(get_twi_address(iface), rx_buffer, sizeof(rx_buffer));
    }
    if (r == false) {
        NRF_LOG_ERROR("hal_i2c_wake failed: ATCA_RX_TIMEOUT");
        return ATCA_RX_TIMEOUT;
    }

    // 5. Set frequency back to requested one
    const uint8_t expected_response[4]  = {0x04, 0x11, 0x33, 0x43};
    const uint8_t selftest_fail_resp[4] = {0x04, 0x07, 0xC4, 0x40};

    if (memcmp(rx_buffer, expected_response, sizeof(expected_response)) == 0) {
        return ATCA_SUCCESS;

    } else if (memcmp(rx_buffer, selftest_fail_resp, sizeof(selftest_fail_resp)) == 0) {
        NRF_LOG_ERROR("hal_i2c_wake failed: selftest error");
        return ATCA_STATUS_SELFTEST_ERROR;

    } else {
        NRF_LOG_ERROR("hal_i2c_wake failed: unknown error");
        return ATCA_WAKE_FAILED;
    }
}

ATCA_STATUS hal_i2c_idle(ATCAIface iface) 
{
    // idle word address value
    uint8_t buffer[1] = {0x2};
    fido_twi_write(get_twi_address(iface), buffer, sizeof(buffer));

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_sleep(ATCAIface iface) 
{
    // sleep word address value
    uint8_t buffer[1] = {0x1};
    fido_twi_write(get_twi_address(iface), buffer, sizeof(buffer));

    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_release(void *hal_data) {
    UNUSED_PARAMETER(hal_data);
    return ATCA_SUCCESS;
}

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses) {
    UNUSED_PARAMETER(i2c_buses);
    UNUSED_PARAMETER(max_buses);
    return ATCA_UNIMPLEMENTED;
}

ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg *cfg, int *found) {
    UNUSED_PARAMETER(bus_num);
    UNUSED_PARAMETER(cfg);
    UNUSED_PARAMETER(found);
    return ATCA_UNIMPLEMENTED;
}

void atca_delay_us(uint32_t delay)
{
    nrf_delay_us(delay);
}

void atca_delay_10us(uint32_t delay)
{
    nrf_delay_us(delay * 10);
}

void atca_delay_ms(uint32_t delay)
{
    nrf_delay_ms(delay);
}
