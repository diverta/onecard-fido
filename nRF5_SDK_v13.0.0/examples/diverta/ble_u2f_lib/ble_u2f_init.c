#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>

#include "ble_u2f.h"
#include "ble_u2f_pairing.h"
#include "nrf_log.h"

#define BLE_UUID_U2F_CONTROL_POINT_CHAR             0xFFF1
#define BLE_UUID_U2F_STATUS_CHAR                    0xFFF2
#define BLE_UUID_U2F_CONTROL_POINT_LENGTH_CHAR      0xFFF3
#define BLE_UUID_U2F_SERVICE_REVISION_BITFIELD_CHAR 0xFFF4
#define BLE_UUID_U2F_SERVICE_REVISION_CHAR          0x2A28

// U2F vendor defined UUID (F1D00000-DEAA-ECEE-B42F-C9BA7ED623BB)
static ble_uuid128_t u2f_base_uuid = {
    0xBB, 0x23, 0xD6, 0x7E, 0xBA, 0xC9, 0x2F, 0xB4, 0xEE, 0xEC, 0xAA, 0xDE, 0x00, 0x00, 0xD0, 0xF1
};

// ペアリング標識はオリジナルUUIDを採用
// (98439EE6-776B-401C-880C-682FBDDD8E32)
#define BLE_UUID_U2F_PAIRING_MODE_CHAR 0x9EE6
static ble_uuid128_t original_base_uuid = {
    0x32, 0x8E, 0xDD, 0xBD, 0x2F, 0x68, 0x0C, 0x88, 0x1C, 0x40, 0x6B, 0x77, 0x00, 0x00, 0x43, 0x98
};
static uint8_t pairing_mode_sign = 0x01;

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


static uint32_t pairing_mode_sign_char_add(ble_u2f_t *p_u2f)
{
    // 'Pairing Mode Sign' characteristicを登録する。
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

    // オリジナルUUID(128bit)を設定
    uint8_t  uuid_type_;
    uint32_t err_code = sd_ble_uuid_vs_add(&original_base_uuid, &uuid_type_);
    VERIFY_SUCCESS(err_code);
    ble_uuid.type = uuid_type_;
    ble_uuid.uuid = BLE_UUID_U2F_PAIRING_MODE_CHAR;

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
    attr_char_value.init_len  = sizeof(pairing_mode_sign);
    attr_char_value.init_offs = 0;
    attr_char_value.max_len   = sizeof(pairing_mode_sign);
    attr_char_value.p_value   = &pairing_mode_sign;

    // U2Fサービス内に追加
    return sd_ble_gatts_characteristic_add(p_u2f->service_handle,
                                           &char_md,
                                           &attr_char_value,
                                           &p_u2f->pairing_mode_sign_handles);
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

    if (ble_u2f_pairing_mode_get()) {
        // ペアリングモードの場合はペアリングモード標識を追加
        err_code = pairing_mode_sign_char_add(p_u2f);
        VERIFY_SUCCESS(err_code);

    } else {
        // 非ペアリングモードの場合はU2F関連キャラクタリスティックを追加
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
    }
    
    return NRF_SUCCESS;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
