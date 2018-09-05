#include "mbed.h"
#include "U2FHID.h"
#include "U2FProcessState.h"

//
// PC-->mbed: 64バイト
// mbed-->PC: 64バイト
USBU2FAuthenticator u2fAuthenticator(HID_PACKET_SIZE, HID_PACKET_SIZE);

//
// HID送信／受信パケット格納領域
//
HID_REPORT send_report;
HID_REPORT recv_report;

int main(void) 
{
    // 初期処理
    u2f_process_state_init();

    while (true) {
        if (u2fAuthenticator.readNB(&recv_report)) {
            if (receive_request_data() == true) {
                // リクエストを全て受領したらレスポンス
                send_response_packet();
            }
        }
        if (u2fAuthenticator.readNB2(&recv_report)) {
            if (receive_request_data() == true) {
                // リクエストを全て受領したらレスポンス
                send_xfer_response_packet();
            }
        }
        // ステータスに応じた処理をメインスレッドで実行
        u2f_process_state_main();
        wait(0.01);
    }
}
