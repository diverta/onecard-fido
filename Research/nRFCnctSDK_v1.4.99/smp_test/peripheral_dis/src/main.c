/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <settings/settings.h>

#ifndef ORIGINAL_SOURCE
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"
#include <mgmt/mcumgr/smp_bt.h>
#endif

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_DIS_VAL)),
#ifndef ORIGINAL_SOURCE
	BT_DATA_BYTES(BT_DATA_UUID128_SOME,
		0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
		0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
#endif
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err 0x%02x)\n", err);
	} else {
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason 0x%02x)\n", reason);
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static int settings_runtime_load(void)
{
#if defined(CONFIG_BT_DIS_SETTINGS)
	settings_runtime_set("bt/dis/model",
			     "Zephyr Model",
			     sizeof("Zephyr Model"));
	settings_runtime_set("bt/dis/manuf",
			     "Zephyr Manufacturer",
			     sizeof("Zephyr Manufacturer"));
#if defined(CONFIG_BT_DIS_SERIAL_NUMBER)
	settings_runtime_set("bt/dis/serial",
			     CONFIG_BT_DIS_SERIAL_NUMBER_STR,
			     sizeof(CONFIG_BT_DIS_SERIAL_NUMBER_STR));
#endif
#if defined(CONFIG_BT_DIS_SW_REV)
	settings_runtime_set("bt/dis/sw",
			     CONFIG_BT_DIS_SW_REV_STR,
			     sizeof(CONFIG_BT_DIS_SW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_FW_REV)
	settings_runtime_set("bt/dis/fw",
			     CONFIG_BT_DIS_FW_REV_STR,
			     sizeof(CONFIG_BT_DIS_FW_REV_STR));
#endif
#if defined(CONFIG_BT_DIS_HW_REV)
	settings_runtime_set("bt/dis/hw",
			     CONFIG_BT_DIS_HW_REV_STR,
			     sizeof(CONFIG_BT_DIS_HW_REV_STR));
#endif
#endif
	return 0;
}

void main(void)
{
	int err;
	
#ifndef ORIGINAL_SOURCE
	os_mgmt_register_group();
	img_mgmt_register_group();
#endif
	
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) {
		settings_load();
	}

	settings_runtime_load();

	printk("Bluetooth initialized\n");

	bt_conn_cb_register(&conn_callbacks);
	
#ifndef ORIGINAL_SOURCE
	// Initialize the Bluetooth mcumgr transport.
	err = smp_bt_register();
	if (err != 0) {
		printk("smp_bt_register failed (err %d)", err);
		return;
	}
#endif
	
	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	printk("Advertising successfully started\n");
}
