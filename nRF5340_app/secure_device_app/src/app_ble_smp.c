/* 
 * File:   app_ble_smp.c
 * Author: makmorit
 *
 * Created on 2021/08/09, 14:59
 */
#include <stdbool.h>

#ifdef CONFIG_MCUMGR_SMP_BT
//
// for Bluetooth smp service
//
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/mgmt/mcumgr/smp_bt.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

void app_ble_smp_register_group(void)
{
    // BLE SMPサービスの設定
    os_mgmt_register_group();
    img_mgmt_register_group();
}

void app_ble_smp_bt_register(void)
{
    // Initialize the Bluetooth mcumgr transport.
    smp_bt_register();
}

#else

bool app_ble_smp_ad_uuid_set(void *data)
{
    (void)data;
    return false;
}

void app_ble_smp_register_group(void)
{
}

void app_ble_smp_bt_register(void)
{
}

#endif
