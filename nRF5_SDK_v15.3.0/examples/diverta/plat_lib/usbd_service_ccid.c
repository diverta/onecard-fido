/* 
 * File:   usbd_service_ccid.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_error.h"

//
// CCID関連
//
#include "usbd_service.h"
#include "usbd_service_ccid.h"
#include "app_usbd_ccid.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// Extended APDUフォーマットに対応するための一時バッファ
//   1208バイトを上限とします。
//
static uint8_t bulkout_data[1280];
static uint8_t bulkin_data[1280];

// データ受信用一時変数
static size_t apdu_size_received;
static size_t apdu_size_expected;
static volatile uint8_t bulkout_state;

static void clear_value(void)
{
    // モジュール変数を初期化
    apdu_size_received = 0;
    apdu_size_expected = 0;
    bulkout_state = 0;
}

// ATR
// Fi=372, Di=1, 372 cycles/ETU 10752 bits/s at 4.00 MHz
// BWT = 5.7s
static const uint8_t atr_ccid[] = {
    0x3B, 0xF7, 0x11, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x65, 
    0x43, 0x61, 0x6E, 0x6F, 0x6B, 0x65, 0x79, 
    0x99};

//
// APDU処理（仮実装）
//
static uint32_t le_bytes_to_hex32(uint8_t *data)
{
    uint32_t u;
    memcpy(&u, data, 4);
    return u;
}

static void hex32_to_le_bytes(uint32_t u, uint8_t *data)
{
    memcpy(data, &u, 4);
}

static void icc_power_on(uint8_t *received_data)
{
    // レスポンス編集
    // bMessageType
    bulkin_data[0] = RDR_TO_PC_DATABLOCK;

    // dwLength
    uint32_t u = sizeof(atr_ccid);
    hex32_to_le_bytes(u, bulkin_data + 1);
    // APDU
    uint8_t const *apdu = atr_ccid;
    size_t apdu_size = sizeof(atr_ccid);
    memcpy(bulkin_data + CCID_CMD_HEADER_SIZE, apdu, apdu_size);

    // bStatus
    // BM_COMMAND_STATUS_NO_ERROR(0x00) | BM_ICC_PRESENT_ACTIVE(0x00)
    bulkin_data[7] = (0x00 | 0x00);
    // bError
    bulkin_data[8] = SLOT_NO_ERROR;
    // bSpecific
    bulkin_data[9] = 0;

    // レスポンス送信
    usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE + apdu_size);
}

static void get_slot_status(uint8_t error, uint8_t *received_data)
{
    // レスポンス編集
    // bMessageType
    bulkin_data[0] = RDR_TO_PC_SLOTSTATUS;
    // dwLength
    uint32_t u = 0;
    hex32_to_le_bytes(u, bulkin_data + 1);

    // bStatus
    // BM_COMMAND_STATUS_NO_ERROR(0x00) | BM_ICC_PRESENT_ACTIVE(0x00)
    bulkin_data[7] = (0x00 | 0x00);
    // bError
    bulkin_data[8] = error;
    // bSpecific
    bulkin_data[9] = 0;

    // レスポンス送信
    if (received_data[6] < 0x05) {
        usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE);
    }
}

static void apdu_received(void)
{
    uint8_t bMessageType = bulkout_data[0];
    switch (bMessageType) {
        case PC_TO_RDR_ICCPOWERON:
            NRF_LOG_DEBUG("Slot power on");
            icc_power_on(bulkout_data);
            break;
        case PC_TO_RDR_ICCPOWEROFF:
            // TODO: 後日実装
            NRF_LOG_DEBUG("Slot power off");
            break;
        case PC_TO_RDR_GETSLOTSTATUS:
            get_slot_status(SLOT_NO_ERROR, bulkout_data);
            break;
        case PC_TO_RDR_XFRBLOCK:
            // TODO: 後日実装
            NRF_LOG_DEBUG("PC_TO_RDR_XFRBLOCK requested");
            break;
        case PC_TO_RDR_GETPARAMETERS:
            // TODO: 後日実装
            NRF_LOG_DEBUG("Slot get param");
            break;
        default:
            get_slot_status(SLOTERROR_CMD_NOT_SUPPORTED, bulkout_data);
            break;
    }
}

static void usbd_ccid_data_frame_received(void)
{
    // 受信フレームデータを取得
    uint8_t *data = app_usbd_ccid_ep_output_buffer();
    size_t len = app_usbd_ccid_ep_output_buffer_size();

    NRF_LOG_DEBUG("usbd_ccid_data_frame_received(%d bytes):", len);
    NRF_LOG_HEXDUMP_DEBUG(data, len);

    // APDU格納領域
    uint8_t *apdu_data = bulkout_data + CCID_CMD_HEADER_SIZE;

    // 初回フレームの場合
    if (bulkout_state == 0) {
        if (len < CCID_CMD_HEADER_SIZE) {
            // 受信フレームにヘッダー（10バイト）が含まれていない場合は終了
            return;
        }

        // フレームデータをバッファに退避
        apdu_size_received = len - CCID_CMD_HEADER_SIZE;
        memcpy(bulkout_data, data, CCID_CMD_HEADER_SIZE);
        memcpy(apdu_data, data + CCID_CMD_HEADER_SIZE, apdu_size_received);

        // dwLength Offset = 1 (4 bytes)
        apdu_size_expected = (size_t)le_bytes_to_hex32(bulkout_data + 1);

        // 送信データ領域を初期化
        memset(bulkin_data, 0, sizeof(bulkin_data));
        
        // 送信データ領域に初期値を設定
        // bSlot Offset = 5
        // bSeq  Offset = 6
        bulkin_data[5] = bulkout_data[5];
        bulkin_data[6] = bulkout_data[6];

        if (apdu_size_received == apdu_size_expected) {
            // APDUの処理を実行
            apdu_received();

        } else if (apdu_size_received < apdu_size_expected) {
            if (apdu_size_expected > ABDATA_SIZE) {
                // 受信データは無効
                bulkout_state = 0;
            } else {
                // 継続フレームを受信
                bulkout_state = 1;
            }
        } else {
            // 無効とする
            bulkout_state = 0;
        }

    // 継続フレームの場合
    } else if (bulkout_state == 1) {
        if (apdu_size_received + len < apdu_size_expected) {
            memcpy(apdu_data + apdu_size_received, data, len);
            apdu_size_received += len;

        } else if (apdu_size_received + len == apdu_size_expected) {
            memcpy(apdu_data + apdu_size_received, data, len);
            bulkout_state = 0;
            // APDUの処理を実行
            apdu_received();

        } else {
            bulkout_state = 0;
        }
    }
}

static void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event)
{
    switch (event) {
        case APP_USBD_CCID_USER_EVT_RX_DONE:
            usbd_ccid_data_frame_received();
            break;
        default:
            break;
    }
}

APP_USBD_CCID_GLOBAL_DEF(m_app_inst_ccid, ccid_user_ev_handler, CCID_DATA_INTERFACE, CCID_DATA_EPIN, CCID_DATA_EPOUT);

void usbd_ccid_init(void)
{
    app_usbd_class_inst_t const * class_ccid = app_usbd_ccid_class_inst_get(&m_app_inst_ccid);
    ret_code_t ret = app_usbd_class_append(class_ccid);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_class_append(class_ccid) returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    // モジュール変数を初期化
    clear_value();
    NRF_LOG_DEBUG("usbd_ccid_init() done");
}

void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size)
{
    app_usbd_ccid_ep_input_from_buffer(p_data, size);

    NRF_LOG_DEBUG("usbd_ccid_send_data_frame(%d bytes)", size);
    NRF_LOG_HEXDUMP_DEBUG(p_data, size);
}
