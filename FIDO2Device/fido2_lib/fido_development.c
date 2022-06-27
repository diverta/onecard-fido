/* 
 * File:   fido_development.c
 * Author: makmorit
 *
 * Created on 2022/06/27, 10:33
 */
//
// プラットフォーム非依存コード
//
#include "fido_hid_receive.h"
#include "fido_hid_send.h"
#include "fido_development.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_development);
#endif

// トランスポート種別を保持
static TRANSPORT_TYPE m_transport_type;

static uint8_t get_command_byte(void)
{
    uint8_t cmd;
    switch (m_transport_type) {
        case TRANSPORT_HID:
            cmd = fido_hid_receive_header()->CMD;
            break;
        default:
            cmd = 0x00;
            break;
    }
    return cmd;
}

void fido_development_command(TRANSPORT_TYPE transport_type)
{
    // トランスポート種別を保持
    m_transport_type = transport_type;

    // リクエストデータ受信後に実行すべき処理を判定
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        default:
            break;
    }

    // LEDをビジー状態に遷移
    fido_status_indicator_busy();
}

void fido_development_command_report_sent(void)
{
    // 全フレーム送信後に行われる後続処理を実行
    uint8_t cmd = get_command_byte();
    switch (cmd) {
        default:
            break;
    }

    // LEDをアイドル状態に遷移
    fido_status_indicator_idle();
}
