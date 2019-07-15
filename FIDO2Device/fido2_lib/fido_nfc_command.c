/* 
 * File:   fido_nfc_command.c
 * Author: makmorit
 *
 * Created on 2019/06/03, 15:20
 */
#include <stdbool.h>
//
// プラットフォーム非依存コード
//
#include "fido_ctap2_command.h"
#include "fido_nfc_receive.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

void fido_nfc_command_on_send_completed(void)
{
    // FIDO機能レスポンスの
    // 全フレーム送信完了時の処理を実行
    //
    // 処理タイムアウト監視を停止
    fido_process_timeout_timer_stop();

    // 全フレーム送信後に行われる後続処理を実行
    fido_ctap2_command_cbor_response_sent();

    if (fido_command_do_abort()) {
        // レスポンス完了後の処理を停止させる場合はここで終了
        return;
    }

    // LED制御をアイドル中（秒間２回点滅）に変更
    fido_status_indicator_idle();
}

void fido_nfc_command_on_request_started(void) 
{
    // FIDO機能リクエストの
    // 先頭フレーム受信時の処理を実行
    // 
    // 処理タイムアウト監視を開始
    fido_process_timeout_timer_start(PROCESS_TIMEOUT_MSEC, NULL);

    // LED制御をアイドル中-->非アイドル中に変更
    fido_status_indicator_no_idle();
}
