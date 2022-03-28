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
#include <bluetooth/bluetooth.h>
#include <mgmt/mcumgr/smp_bt.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

static struct bt_data ad_uuid_smp = BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86, 0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d);

bool app_ble_smp_ad_uuid_set(void *data)
{
    struct bt_data *p_bt_data = (struct bt_data *)data;
    *p_bt_data = ad_uuid_smp;
    return true;
}

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
