/* 
 * File:   ccid_main.c
 * Author: makmorit
 *
 * Created on 2020/05/29, 12:37
 */
#include "ccid.h"
#include "ccid_apdu.h"
#include "ccid_process.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_main);
#endif

//
// 送受信データ格納用の一時バッファ
// 受信用は、以下340バイト分を確保
//   最大受信可能データ長（330バイト）
//   コマンドヘッダー長（10バイト）
//
static uint8_t bulkout_data[340];
static uint8_t bulkin_data[288];

// データ受信用一時変数
static size_t bulkout_size_received;
static size_t bulkout_size_expected;

// 読込中の状態をあらわすフラグ
//   0:先頭フレーム受信待ち
//   1:継続フレーム受信待ち
static uint8_t bulkout_state;

void ccid_initialize_value(void)
{
    // モジュール変数を初期化
    bulkout_size_received = 0;
    bulkout_size_expected = 0;
    bulkout_state = 0;
}

// ATR
// Fi=372, Di=1, 372 cycles/ETU 10752 bits/s at 4.00 MHz
// BWT = 5.7s
static uint8_t atr_ccid[] = {
    0x3b, 0xfd, 0x11, 0x00, 0x00, 0x81, 0x31, 0xfe, 0x65, 
    0x53, 0x65, 0x63, 0x75, 0x72, 0x65, 0x20, 0x44, 0x6f, 0x6e, 0x67, 0x6c, 0x65,
    0xfb};

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

uint8_t *ccid_command_apdu_data(void)
{
    // 受信APDUの先頭アドレスを戻す
    return bulkout_data + CCID_CMD_HEADER_SIZE;
}

size_t ccid_command_apdu_size(void)
{
    // 受信APDUのデータ長を戻す
    size_t apdu_size = (size_t)le_bytes_to_hex32(bulkout_data + 1);
    return apdu_size;
}

uint8_t *ccid_response_apdu_data(void)
{
    // 送信APDUの先頭アドレスを戻す
    return bulkin_data + CCID_CMD_HEADER_SIZE;
}

void ccid_response_apdu_size_set(size_t size)
{
    // 送信APDUのデータ長を設定
    set_bulkin_data_dw_length(size);
}

size_t ccid_response_apdu_size_max(void)
{
    // 送信APDUの最大格納可能データ長を設定
    return (sizeof(bulkin_data) - CCID_CMD_HEADER_SIZE);
}

//
// リクエスト処理（PC --> Reader）
//
static uint8_t pc_to_reader_icc_power_on(void) 
{
    fido_log_debug("Slot power on");

    // bSpecific_0
    uint8_t voltage = bulkout_data[7];
    if (voltage != 0x00) {
        // dwLength
        set_bulkin_data_dw_length(0);
        // bStatus
        set_bulkin_data_status(BM_COMMAND_STATUS_FAILED, BM_ICC_PRESENT_ACTIVE);
        fido_log_error("pc_to_reader_icc_power_on: Bad power select");
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
    fido_log_debug("Slot power off");

    // Appletを停止
    ccid_process_stop_applet();

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_INACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_get_slot_status(void) 
{
    // fido_log_debug("Slot get status");

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

static uint8_t pc_to_reader_get_parameters(void) 
{
    fido_log_debug("Slot get param");

    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    return SLOT_NO_ERROR;
}

//
// レスポンス処理（Reader --> PC）
//
static void reader_to_pc_slot_status(uint8_t error) 
{
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

void ccid_request_apdu_received(void)
{
    uint8_t error;

    // 送信データ領域に初期値を設定
    // bSlot Offset = 5
    // bSeq  Offset = 6
    memset(bulkin_data, 0, sizeof(bulkin_data));
    bulkin_data[5] = bulkout_data[5];
    bulkin_data[6] = bulkout_data[6];

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
            // 受信APDUに対する処理を実行する
            ccid_apdu_process();
            break;
        case PC_TO_RDR_GETPARAMETERS:
            error = pc_to_reader_get_parameters();
            reader_to_pc_parameters(error);
            break;
        default:
            fido_log_error("Slot error command");
            reader_to_pc_slot_status(SLOTERROR_CMD_NOT_SUPPORTED);
            break;
    }
}

void ccid_resume_reader_to_pc_data_block(void)
{
    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_NO_ERROR, BM_ICC_PRESENT_ACTIVE);
    reader_to_pc_data_block(SLOT_NO_ERROR);
}

bool ccid_data_frame_received(uint8_t *data, size_t len)
{
    // 先頭フレーム待ちの場合
    if (bulkout_state == 0) {
        if (len < CCID_CMD_HEADER_SIZE) {
            // 受信フレームにヘッダー（10バイト）が含まれていない場合は終了
            return false;
        }

        // フレームデータをバッファに退避
        bulkout_size_received = len;
        memcpy(bulkout_data, data, len);

        // dwLength Offset = 1 (4 bytes) 
        memcpy(&bulkout_size_expected, bulkout_data + 1, 4);

        // ブロックサイズを取得して保持
        // Block size = Command header size + APDU size
        bulkout_size_expected += CCID_CMD_HEADER_SIZE;

        if (bulkout_size_expected > sizeof(bulkout_data)) {
            // ブロックサイズが配列サイズを超えている場合は
            // フレーム受信は継続するが、データは無効とする
            bulkout_state = 2;

        } else if (bulkout_size_received == bulkout_size_expected) {
            // ブロックサイズ＝フレームサイズの場合は、ブロック受信を完了し、
            // APDUの処理を実行
            return true;

        } else if (bulkout_size_received < bulkout_size_expected) {
            // フレーム受信を継続
            bulkout_state = 1;

        } else {
            // 無効とする
            bulkout_state = 0;
        }

    // 継続フレーム待ちの場合
    } else if (bulkout_state == 1) {
        if (bulkout_size_received + len < bulkout_size_expected) {
            // 受信済みサイズがブロックサイズに満たない場合は、フレーム受信を継続
            memcpy(bulkout_data + bulkout_size_received, data, len);
            bulkout_size_received += len;

        } else if (bulkout_size_received + len == bulkout_size_expected) {
            // ブロックサイズ通りに受信できた場合は、ブロック受信を完了
            memcpy(bulkout_data + bulkout_size_received, data, len);
            bulkout_state = 0;
            // APDUの処理を実行
            return true;

        } else {
            bulkout_state = 0;
        }

    // ブロックサイズが配列サイズを超えている場合は
    // フレーム受信は継続するが、データは無効とする
    } else if (bulkout_state == 2) {
        if (bulkout_size_received + len < bulkout_size_expected) {
            // 受信済みサイズがブロックサイズに満たない場合は、フレーム受信を継続
            bulkout_size_received += len;
        } else {
            // ブロックサイズ以上受信した場合は、ブロック受信を完了
            fido_log_error("APDU recv: data size(%d) exceeds buffer size(%d)",
                bulkout_size_expected, sizeof(bulkout_data));
            bulkout_state = 0;
        }
    }

    return false;
}

void ccid_response_time_extension(void)
{
    // bMessageType
    bulkin_data[0] = RDR_TO_PC_DATABLOCK;
    // dwLength
    set_bulkin_data_dw_length(0);
    // bStatus
    set_bulkin_data_status(BM_COMMAND_STATUS_TIME_EXTN, BM_ICC_PRESENT_ACTIVE);
    // bError
    // request another 1 BTWs (5.7s)
    bulkin_data[8] = 1;
    // bSpecific
    bulkin_data[9] = 0x00;

    // レスポンス送信
    usbd_ccid_send_data_frame(bulkin_data, CCID_CMD_HEADER_SIZE);
}

//
// 共通関数
//
uint16_t ccid_get_tlv_element_size(uint8_t elem_no, uint8_t *data, size_t size, size_t *elem_header_size, uint16_t *elem_data_size, uint16_t elem_data_size_max) 
{
    if (size < 1) {
        return SW_WRONG_LENGTH;
    }
    if (data[0] != elem_no) {
        return SW_WRONG_DATA;
    }
    
    if (data[1] < 0x80) {
        *elem_data_size = data[1];
        *elem_header_size = 2;

    } else if (data[1] == 0x81) {
        if (size < 2) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_data_size = data[2];
            *elem_header_size = 3;
        }

    } else if (data[1] == 0x82) {
        if (size < 3) {
            return SW_WRONG_LENGTH;

        } else {
            *elem_data_size = (uint16_t)(data[2] << 8u) | data[3];
            *elem_header_size = 4;
        }

    } else {
        return SW_WRONG_LENGTH;
    }

    if (*elem_data_size + *elem_header_size > size) {
        // length does not overflow, but data does overflow
        return SW_WRONG_LENGTH;
    }

    if (*elem_data_size > elem_data_size_max) {
        return SW_WRONG_DATA;
    }

    return SW_NO_ERROR;
}

void ccid_assert_apdu(void *p_capdu, void *p_rapdu)
{
    if (p_capdu == NULL || p_rapdu == NULL) {
        while(true);
    }
}
