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

// 読込中の状態をあらわすフラグ
//   0:先頭フレーム受信待ち
//   1:継続フレーム受信待ち
static uint8_t bulkout_state;

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
static uint8_t atr_ccid[] = {
    0x3B, 0xF7, 0x11, 0x00, 0x00, 0x81, 0x31, 0xFE, 0x65, 
    0x43, 0x61, 0x6E, 0x6F, 0x6B, 0x65, 0x79, 
    0x99};

//
// 各種業務処理（仮実装）
//
void applet_poweroff(void)
{
}

void process_apdu(void)
{
}

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

static void set_bulkin_data_apdu(uint8_t *data, size_t size)
{
    // APDU
    uint8_t *apdu = bulkin_data + CCID_CMD_HEADER_SIZE;
    memcpy(apdu, data, size);
}

static void set_bulkin_data_dw_length(size_t size)
{
    // dwLength
    uint32_t u = (uint32_t)size;
    hex32_to_le_bytes(u, bulkin_data + 1);
}

static void set_bulkin_data_status(uint8_t cmd_status, uint8_t icc_status)
{
    // bStatus
    bulkin_data[7] = (cmd_status | icc_status);
}

uint8_t *usbd_ccid_command_apdu_data(void)
{
    // 受信APDUの先頭アドレスを戻す
    return bulkout_data + CCID_CMD_HEADER_SIZE;
}

size_t usbd_ccid_command_apdu_size(void)
{
    // 受信APDUのデータ超を戻す
    size_t apdu_size = (size_t)le_bytes_to_hex32(bulkout_data + 1);
    return apdu_size;
}

//
// リクエスト処理（PC --> Reader）
//
static uint8_t pc_to_reader_icc_power_on(void) 
{
    NRF_LOG_DEBUG("Slot power on");

    // bSpecific_0
    uint8_t voltage = bulkout_data[7];
    if (voltage != 0x00) {
        // dwLength
        set_bulkin_data_dw_length(0);
        // bStatus
        set_bulkin_data_status(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        NRF_LOG_ERROR("Bad power select");
        return SLOTERROR_BAD_POWERSELECT;
    }

    // APDU
    set_bulkin_data_apdu(atr_ccid, sizeof(atr_ccid));
    // dwLength
    set_bulkin_data_dw_length(sizeof(atr_ccid));
    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_icc_power_off(void) 
{
    NRF_LOG_DEBUG("Slot power off");

    // Appletを停止
    applet_poweroff();

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_INACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_get_slot_status(void) 
{
    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_xfr_block(void)
{
    // 受信APDUに対する処理を実行する
    process_apdu();

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_get_parameters(void) 
{
    NRF_LOG_DEBUG("Slot get param");

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

//
// レスポンス処理（Reader --> PC）
//
static void reader_to_pc_slot_status(uint8_t error) 
{
    // これは仮の処理です。
    if (bulkout_data[6] > 0x05) {
        return;
    }
    
    // bMessageType
    bulkin_data[0] = RDR_TO_PC_SLOTSTATUS;
    // dwLength
    set_bulkin_data_dw_length(0);
    // bError
    bulkin_data[8] = error;
    // bSpecific
    bulkin_data[9] = 0;
    
    // レスポンス送信
    usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE);
}

static void reader_to_pc_data_block(uint8_t error) 
{
    // bMessageType
    bulkin_data[0] = RDR_TO_PC_DATABLOCK;
    // bError
    bulkin_data[8] = error;
    // bSpecific
    bulkin_data[9] = 0;

    // レスポンス送信
    size_t apdu_size = (size_t)le_bytes_to_hex32(bulkin_data + 1);
    usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE + apdu_size);
}

static void reader_to_pc_parameters(uint8_t error) 
{
    // dwLength
    if (error == SLOT_NO_ERROR) {
        set_bulkin_data_dw_length(7);
    } else {
        set_bulkin_data_dw_length(0);
    }

    // APDU
    uint8_t *apdu = bulkin_data + CCID_CMD_HEADER_SIZE;
    apdu[0] = 0x11; // Fi=372, Di=1
    apdu[1] = 0x10; // Checksum: LRC, Convention: direct, ignored by CCID
    apdu[2] = 0x00; // No extra guard time
    apdu[3] = 0x15; // BWI = 1, CWI = 5
    apdu[4] = 0x00; // Stopping the Clock is not allowed
    apdu[5] = 0xFE; // IFSC = 0xFE
    apdu[6] = 0x00; // NAD

    // bMessageType
    bulkin_data[0] = RDR_TO_PC_PARAMETERS;
    // bError
    bulkin_data[8] = error;
    // bSpecific
    bulkin_data[9] = 0x01;

    // レスポンス送信
    size_t apdu_size = (size_t)le_bytes_to_hex32(bulkin_data + 1);
    usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE + apdu_size);
}

static void apdu_received(void)
{
    uint8_t error;

    // bMessageType で処理分岐
    switch (bulkout_data[0]) {
        case PC_TO_RDR_ICCPOWERON:
            error = pc_to_reader_icc_power_on();
            reader_to_pc_data_block(error);
            break;
        case PC_TO_RDR_ICCPOWEROFF:
            error = pc_to_reader_icc_power_off();
            reader_to_pc_slot_status(error);
            break;
        case PC_TO_RDR_GETSLOTSTATUS:
            error = pc_to_reader_get_slot_status();
            reader_to_pc_slot_status(error);
            break;
        case PC_TO_RDR_XFRBLOCK:
            error = pc_to_reader_xfr_block();
            reader_to_pc_data_block(error);
            break;
        case PC_TO_RDR_GETPARAMETERS:
            error = pc_to_reader_get_parameters();
            reader_to_pc_parameters(error);
            break;
        default:
            reader_to_pc_slot_status(SLOTERROR_CMD_NOT_SUPPORTED);
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

    // 先頭フレーム待ちの場合
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

    // 継続フレーム待ちの場合
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
