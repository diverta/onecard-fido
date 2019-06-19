/* 
 * File:   fido_flash_event.h
 * Author: makmorit
 *
 * Created on 2019/06/19, 10:10
 */
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_ctap2_command.h"
#include "fido_u2f_command.h"
#include "fido_ctap2_command.h"
#include "hid_fido_command.h"
#include "nfc_fido_command.h"
#include "fido_ble_main.h"

#include "fido_flash.h"
#include "fido_flash_event.h"

//
// FDSから渡されたイベント情報を保持
//
static fido_flash_event_t flash_event;

static void fido_command_on_fs_evt(fds_evt_t const *const p_evt)
{
    // FDS処理完了後のBLE処理を実行
    ble_u2f_command_on_fs_evt(p_evt);

    // 処理結果を構造体に保持
    flash_event.result = (p_evt->result == FDS_SUCCESS);
    flash_event.gc = (p_evt->id == FDS_EVT_GC);
    flash_event.delete_file = (p_evt->id == FDS_EVT_DEL_FILE);
    flash_event.write_update = (p_evt->id == FDS_EVT_UPDATE || p_evt->id == FDS_EVT_WRITE);
    flash_event.retry_counter_write = (p_evt->write.record_key == FIDO_PIN_RETRY_COUNTER_RECORD_KEY);

    // FDS処理完了後のUSB HID処理を実行
    hid_fido_command_on_fs_evt(&flash_event);

    // FDS処理完了後のNFC処理を実行
    nfc_fido_command_on_fs_evt(&flash_event);
}

void fido_command_fds_register(void)
{
    // FDS処理完了後の処理をFDSに登録
    ret_code_t err_code = fds_register(fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}
