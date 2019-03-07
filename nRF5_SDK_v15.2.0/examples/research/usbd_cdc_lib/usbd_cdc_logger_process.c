/* 
 * File:   usbd_cdc_logger_process.c
 * Author: makmorit
 *
 * Created on 2019/03/05, 12:53
 */
#include "sdk_common.h"

#include "nrf_drv_usbd.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_cdc_logger_process
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for output log text
#include "usbd_cdc_service.h"

// for usbd_cdc_logger_serial_second
#include "usbd_cdc_logger_interval_timer.h"

// for get_adv_stat_info
#include "fido_ble_central.h"

// 秒通番を保持
static uint32_t curr_serial_second = 0;

// 編集用領域
static char m_tx_buffer[128];

void usbd_cdc_logger_process(void)
{
    uint32_t   serial_second;
    size_t     size;
    
    // 秒通番が変わったらログ出力処理を行う
    serial_second = usbd_cdc_logger_serial_second();
    if (curr_serial_second == serial_second) {
        return;
    }

    // 秒通番を待避
    curr_serial_second = serial_second;

    if (!nrf_drv_usbd_is_started()) {
        // PCのUSBポートに装着されていない場合
        // NRF_LOG_ERROR("Device not connected to USB port: serial second(%lu)", curr_serial_second);
        return;
    }

    // ログを編集して出力
    size = get_adv_stat_info_string(curr_serial_second, m_tx_buffer);
    if (usbd_cdc_buffer_write(m_tx_buffer, size) == false) {
        return;
    }
}
