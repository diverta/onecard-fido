/* 
 * File:   fido_ble_main.c
 * Author: makmorit
 *
 * Created on 2018/10/08, 10:37
 */
// for ble_advertising_init_t
#include "ble_advertising.h"

// for logging informations
#define NRF_LOG_MODULE_NAME fido_ble_main
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for FIDO
#include "ble_u2f.h"
#include "ble_u2f_command.h"
#include "ble_u2f_pairing.h"
#include "ble_u2f_init.h"

// for fds_register on hid service
#include "hid_fido_command.h"

//
// U2F関連の共有情報
//
static ble_u2f_t m_u2f;

void fido_ble_advertising_init(ble_advertising_init_t *p_init)
{
    // アドバタイジング設定の前に、
    // ペアリングモードをFDSから取得
    ble_u2f_pairing_get_mode(&m_u2f);
    
    // ペアリングモードでない場合は、
    // ディスカバリーができないよう設定
    p_init->advdata.flags = ble_u2f_pairing_advertising_flag();
}

//
// BLEサービス初期化
//
void fido_ble_services_init(void)
{
    // U2Fサービスを初期化
    ble_u2f_init_services(&m_u2f);
}

void fido_ble_peer_manager_init(void)
{
    // FDS処理完了後のU2F処理を続行させる
    ret_code_t err_code = fds_register(ble_u2f_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);

    err_code = fds_register(hid_fido_command_on_fs_evt);
    APP_ERROR_CHECK(err_code);
}

ble_u2f_t *fido_ble_get_U2F_context(void)
{
    // U2F関連の共有情報
    return &m_u2f;
}
