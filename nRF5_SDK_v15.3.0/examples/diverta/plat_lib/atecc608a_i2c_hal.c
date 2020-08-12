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
#include "atecc_iface.h"

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

ATECC_STATUS hal_i2c_init(ATECC_IFACE iface) 
{
    if (fido_twi_init() == false) {
        return ATECC_COMM_FAIL;
    }

    ATECC_IFACE_CFG *cfg = iface->mIfaceCFG;
    m_hal_data.active = true;
    m_hal_data.slave_address = cfg->i2c.slave_address;
    m_hal_data.bus = cfg->i2c.bus;
    m_hal_data.baud = cfg->i2c.baud;
    m_hal_data.wake_delay = cfg->wake_delay;
    m_hal_data.rx_retries = cfg->rx_retries;
    m_hal_data.p_instance = (nrf_drv_twi_t const *)fido_twi_instance_ref();
    iface->hal_data = &m_hal_data;

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_i2c_post_init(ATECC_IFACE iface) {
    (void)iface;
    return ATECC_SUCCESS;
}

static uint8_t get_twi_address(ATECC_IFACE iface)
{
    // 7bitアドレスを取得
    i2c_hal_data_t *hal_data = (i2c_hal_data_t *)(iface->hal_data);
    uint8_t address = (hal_data->slave_address >> 1) & 0x7f;
    return address;
}

ATECC_STATUS hal_i2c_send(ATECC_IFACE iface, uint8_t *data, int length) 
{
    // 先頭バイトを差替えて送信
    data[0] = 0x3;
    length++;
    if (fido_twi_write(get_twi_address(iface), data, length) == false) {
        NRF_LOG_ERROR("hal_i2c_send failed");
        return ATECC_TX_FAIL;
    }

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_i2c_receive(ATECC_IFACE iface, uint8_t *rx_data, uint16_t *rx_length) 
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
        return ATECC_RX_TIMEOUT;
    }

    // NACKの場合（応答データが受信できなかった場合）
    if (length_package[0] == 0) {
        NRF_LOG_WARNING("hal_i2c_receive: NACK has occurred");
        *rx_length = 0;
        return ATECC_SUCCESS;
    }
    
    // データの１バイト目に、受信できるバイト数が格納されています
    uint8_t bytes_to_read = length_package[0] - 1;
    if (bytes_to_read > *rx_length) {
        NRF_LOG_ERROR("hal_i2c_receive buffer too small, requested %u, but have %u", bytes_to_read, *rx_length);
        return ATECC_SMALL_BUFFER;
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
        return ATECC_RX_TIMEOUT;
    }
    *rx_length = length_package[0];

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_i2c_wake(ATECC_IFACE iface) 
{
    // steps to wake the chip up...
    // 1. switch to 100KHz
    // 2. Send NULL buffer to address 0x0 (NACK)
    fido_twi_verify_nack(get_twi_address(iface));

    // 3. Wait for wake_delay
    i2c_hal_data_t *hal_data = (i2c_hal_data_t*)(iface->hal_data);
    atecc_delay_us(hal_data->wake_delay);

    // 4. Read from normal slave_address
    uint8_t rx_buffer[4] = {0};
    bool r = false;
    int retries = hal_data->rx_retries;
    while (--retries > 0 && r == false) {
        r = fido_twi_read(get_twi_address(iface), rx_buffer, sizeof(rx_buffer));
    }
    if (r == false) {
        NRF_LOG_ERROR("hal_i2c_wake failed: read timeout");
        return ATECC_RX_TIMEOUT;
    }

    // 5. Set frequency back to requested one
    const uint8_t expected_response[4]  = {0x04, 0x11, 0x33, 0x43};
    const uint8_t selftest_fail_resp[4] = {0x04, 0x07, 0xC4, 0x40};

    if (memcmp(rx_buffer, expected_response, sizeof(expected_response)) == 0) {
        return ATECC_SUCCESS;

    } else if (memcmp(rx_buffer, selftest_fail_resp, sizeof(selftest_fail_resp)) == 0) {
        NRF_LOG_ERROR("hal_i2c_wake failed: selftest error");
        return ATECC_STATUS_SELFTEST_ERROR;

    } else {
        NRF_LOG_ERROR("hal_i2c_wake failed: unknown error");
        return ATECC_WAKE_FAILED;
    }
}

ATECC_STATUS hal_i2c_idle(ATECC_IFACE iface) 
{
    // idle word address value
    uint8_t buffer[1] = {0x2};
    fido_twi_write(get_twi_address(iface), buffer, sizeof(buffer));

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_i2c_sleep(ATECC_IFACE iface) 
{
    // sleep word address value
    uint8_t buffer[1] = {0x1};
    fido_twi_write(get_twi_address(iface), buffer, sizeof(buffer));

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_i2c_release(void *hal_data) {
    UNUSED_PARAMETER(hal_data);
    return ATECC_SUCCESS;
}

ATECC_STATUS hal_iface_init(ATECC_IFACE iface)
{
    if (iface == NULL) {
        return ATECC_COMM_FAIL;
    }

    iface->init_func     = &hal_i2c_init;
    iface->postinit_func = &hal_i2c_post_init;
    iface->send_func     = &hal_i2c_send;
    iface->receive_func  = &hal_i2c_receive;
    iface->wake_func     = &hal_i2c_wake;
    iface->sleep_func    = &hal_i2c_sleep;
    iface->idle_func     = &hal_i2c_idle;
    iface->hal_data      = NULL;

    return ATECC_SUCCESS;
}

ATECC_STATUS hal_iface_release(void *hal_data)
{
    return hal_i2c_release(hal_data);
}

void atecc_delay_us(uint32_t delay)
{
    nrf_delay_us(delay);
}

void atecc_delay_10us(uint32_t delay)
{
    nrf_delay_us(delay * 10);
}

void atecc_delay_ms(uint32_t delay)
{
    nrf_delay_ms(delay);
}
