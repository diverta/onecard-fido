#include "sdk_common.h"

#include <stdio.h>
#include <string.h>

#include "ble_u2f.h"

// for logging informations
#define NRF_LOG_MODULE_NAME ble_u2f_init
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define BLE_UUID_U2F_CONTROL_POINT_CHAR             0xFFF1
#define BLE_UUID_U2F_STATUS_CHAR                    0xFFF2
#define BLE_UUID_U2F_CONTROL_POINT_LENGTH_CHAR      0xFFF3
#define BLE_UUID_U2F_SERVICE_REVISION_BITFIELD_CHAR 0xFFF4
#define BLE_UUID_U2F_SERVICE_REVISION_CHAR          0x2A28

// U2F vendor defined UUID (F1D00000-DEAA-ECEE-B42F-C9BA7ED623BB)
static ble_uuid128_t u2f_base_uuid = {
    {0xBB, 0x23, 0xD6, 0x7E, 0xBA, 0xC9, 0x2F, 0xB4, 0xEE, 0xEC, 0xAA, 0xDE, 0x00, 0x00, 0xD0, 0xF1}
};

// Control Pointバイト長、
// Service Revisionに関する情報を保持
static uint8_t control_point_length[2] = {0x00, 0x40};   // 64Bytes
static uint8_t service_revision_bitfield[1] = {0x80};    // Supports 1.1
static uint8_t service_revision[3] = {0x31, 0x2e, 0x31}; // 1.1


static uint32_t u2f_status_char_add(ble_u2f_t * p_u2f)
{
    // 'U2F Status' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_md_t cccd_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&cccd_md, 0, sizeof(cccd_md));

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&cccd_md.write_perm);

    cccd_md.vloc = BLE_GATTS_VLOC_STACK;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.notify = 1;
    char_md.p_char_user_desc  = NULL;
    char_md.p_char_pf         = NULL;
    char_md.p_user_desc_md    = NULL;
    char_md.p_cccd_md         = &cccd_md;
    char_md.p_sccd_md         = NULL;

    ble_uuid.type = p_u2f->uuid_type;
    ble_uuid.uuid = BLE_UUID_U2F_STATUS_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(uint8_t);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_U2F_MAX_SEND_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_status_handles);
}


static uint32_t u2f_control_point_char_add(ble_u2f_t * p_u2f)
{
    // 'U2F Control Point' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_u2f->uuid_type;
    ble_uuid.uuid = BLE_UUID_U2F_CONTROL_POINT_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 1;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = 1;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = BLE_U2F_MAX_RECV_CHAR_LEN;

    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_control_point_handles);
}


static uint32_t u2f_control_point_length_char_add(ble_u2f_t * p_u2f)
{
    // 'U2F Control Point Length' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_u2f->uuid_type;
    ble_uuid.uuid = BLE_UUID_U2F_CONTROL_POINT_LENGTH_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(control_point_length);;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(control_point_length);
    attr_char_value.p_value   = control_point_length;

    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_control_point_length_handles);
}


static uint32_t u2f_service_revision_bitfield_char_add(ble_u2f_t * p_u2f)
{
    // 'U2F Service Revision Bitfield' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.write = 1;
    char_md.char_props.read          = 1;
    char_md.p_char_user_desc         = NULL;
    char_md.p_char_pf                = NULL;
    char_md.p_user_desc_md           = NULL;
    char_md.p_cccd_md                = NULL;
    char_md.p_sccd_md                = NULL;

    ble_uuid.type = p_u2f->uuid_type;
    ble_uuid.uuid = BLE_UUID_U2F_SERVICE_REVISION_BITFIELD_CHAR;

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 1;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(service_revision_bitfield);;
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(service_revision_bitfield);
    attr_char_value.p_value   = service_revision_bitfield;

    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_service_revision_bitfield_handles);
}


static uint32_t u2f_service_revision_char_add(ble_u2f_t * p_u2f)
{
    // 'U2F Service Revision' characteristicを登録する。
    ble_gatts_char_md_t char_md;
    ble_gatts_attr_t    attr_char_value;
    ble_uuid_t          ble_uuid;
    ble_gatts_attr_md_t attr_md;

    memset(&char_md, 0, sizeof(char_md));

    char_md.char_props.read  = 1;
    char_md.p_char_user_desc = NULL;
    char_md.p_char_pf        = NULL;
    char_md.p_user_desc_md   = NULL;
    char_md.p_cccd_md        = NULL;
    char_md.p_sccd_md        = NULL;

    BLE_UUID_BLE_ASSIGN(ble_uuid, BLE_UUID_U2F_SERVICE_REVISION_CHAR);

    memset(&attr_md, 0, sizeof(attr_md));

    BLE_GAP_CONN_SEC_MODE_SET_ENC_NO_MITM(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&attr_md.write_perm);

    attr_md.vloc    = BLE_GATTS_VLOC_STACK;
    attr_md.rd_auth = 0;
    attr_md.wr_auth = 0;
    attr_md.vlen    = 0;

    memset(&attr_char_value, 0, sizeof(attr_char_value));

    attr_char_value.p_uuid    = &ble_uuid;
    attr_char_value.p_attr_md = &attr_md;
    attr_char_value.init_len  = sizeof(service_revision);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(service_revision);
    attr_char_value.p_value   = service_revision;

    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->u2f_service_revision_handles);
}


uint32_t ble_u2f_init_services(ble_u2f_t * p_u2f)
{
    uint32_t      err_code;
    ble_uuid_t    ble_uuid;

    VERIFY_PARAM_NOT_NULL(p_u2f);

    p_u2f->conn_handle             = BLE_CONN_HANDLE_INVALID;
    p_u2f->data_handler            = NULL;

    ble_uuid.type = BLE_UUID_TYPE_BLE;
    ble_uuid.uuid = BLE_UUID_U2F_SERVICE;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_u2f->service_handle);
    VERIFY_SUCCESS(err_code);

    // U2F vendor defined ベースUUIDを生成
    err_code = sd_ble_uuid_vs_add(&u2f_base_uuid, &p_u2f->uuid_type);
    VERIFY_SUCCESS(err_code);

    err_code = u2f_control_point_char_add(p_u2f);
    VERIFY_SUCCESS(err_code);

    err_code = u2f_status_char_add(p_u2f);
    VERIFY_SUCCESS(err_code);

    err_code = u2f_control_point_length_char_add(p_u2f);
    VERIFY_SUCCESS(err_code);

    err_code = u2f_service_revision_bitfield_char_add(p_u2f);
    VERIFY_SUCCESS(err_code);

    err_code = u2f_service_revision_char_add(p_u2f);
    VERIFY_SUCCESS(err_code);

    return NRF_SUCCESS;
}
