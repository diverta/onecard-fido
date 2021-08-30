/* 
 * File:   app_ble_fido.c
 * Author: makmorit
 *
 * Created on 2021/08/26, 11:50
 */
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "app_ble_fido.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_ble_fido);

// Control Pointバイト長、
// Service Revisionに関する情報を保持
static uint8_t control_point_length[2] = {0x00, 0x40};   // 64Bytes
static uint8_t service_revision_bitfield[1] = {0xe0};    // Supports 1.1, 1.2, 2.0
static uint8_t service_revision[3] = {0x31, 0x2e, 0x31}; // 1.1

static ssize_t read_rx_len(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    // U2F Control Point Length の値を設定
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(control_point_length));
}

static ssize_t read_service_rev_bitfield(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    // U2F Service Revision Bitfield の値を設定
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(service_revision_bitfield));
}

static ssize_t read_service_rev(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    // U2F Service Revision の値を設定
    return bt_gatt_attr_read(conn, attr, buf, len, offset, attr->user_data, sizeof(service_revision));
}

static void ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    // U2F Status(TX) 通知設定変更時の処理
    LOG_DBG("Notification has been turned %s", value == BT_GATT_CCC_NOTIFY ? "on" : "off");
}

static ssize_t on_receive(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    // U2F Status(TX) 書込み時の処理
    LOG_DBG("Received data, handle %d, conn %p", attr->handle, (void *)conn);

    // TODO: 仮の実装です。
    uint8_t testbuf[] = {0x83, 0x00, 0x02, 0x90, 0x00};
    app_ble_fido_send_data(conn, testbuf, sizeof(testbuf));

    return len;
}

static void on_sent(struct bt_conn *conn, void *user_data)
{
    // U2F Control Point(RX) 転送完了時の処理
    (void)user_data;
    LOG_DBG("Data send, conn %p", (void *)conn);
}

// FIDO BLE Service Declaration
BT_GATT_SERVICE_DEFINE(
    fido_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_FIDO_SERVICE),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_FIDO_TX, 
        BT_GATT_CHRC_NOTIFY, 
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, 
        NULL, NULL, NULL
    ),
    BT_GATT_CCC(
        ccc_cfg_changed, 
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    ),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_FIDO_RX, 
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP, 
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE, 
        NULL, on_receive, NULL
    ),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_FIDO_RX_LEN, 
        BT_GATT_CHRC_READ, 
        BT_GATT_PERM_READ, 
        read_rx_len, NULL, control_point_length
    ),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_FIDO_SERVICE_REVBF, 
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_READ, 
        BT_GATT_PERM_READ, 
        read_service_rev_bitfield, NULL, service_revision_bitfield
    ),
    BT_GATT_CHARACTERISTIC(
        BT_UUID_FIDO_SERVICE_REV, 
        BT_GATT_CHRC_READ, 
        BT_GATT_PERM_READ, 
        read_service_rev, NULL, service_revision
    ),
);

int app_ble_fido_send_data(struct bt_conn *conn, const uint8_t *data, uint16_t len)
{
    struct bt_gatt_notify_params params = {0};
    const struct bt_gatt_attr *attr = &fido_svc.attrs[2];

    params.attr = attr;
    params.data = data;
    params.len = len;
    params.func = on_sent;

    if (!conn) {
        LOG_DBG("Notification send to all connected peers");
        return bt_gatt_notify_cb(NULL, &params);

    } else if (bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY)) {
        return bt_gatt_notify_cb(conn, &params);

    } else {
        return -EINVAL;
    }
}
