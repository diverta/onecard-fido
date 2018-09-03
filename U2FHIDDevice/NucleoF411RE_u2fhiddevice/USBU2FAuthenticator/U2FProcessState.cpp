#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include "mbed.h"

#include "U2FHID.h"
#include "U2FProcessState.h"

// LEDを接続するピンを指定
//   緑：PA_5、青：PA_6
DigitalOut LED_GREEN(PA_5);
DigitalOut LED_BLUE(PA_6);

// タイムアウト管理用
static Timeout U2FHelperTimer;
static void onU2FHelperTimeout()
{
    // ステータスを設定
    u2f_process_state_set(U2FPS_HELPER_TIMEOUT);
}

static void u2f_process_state_start_timer()
{
    // タイムアウト判定用のタイマーを開始
    U2FHelperTimer.attach(&onU2FHelperTimeout, 20);
}

static void u2f_process_state_stop_timer()
{
    // タイムアウト判定用のタイマーを停止
    U2FHelperTimer.detach();
}

//
// 処理ステータスを管理
//
U2F_PROCESS_STATE u2f_process_state;
void u2f_process_state_set(U2F_PROCESS_STATE _state)
{
    u2f_process_state = _state;
}

//
// 初期処理
//
void u2f_process_state_init(void)
{
    // デバイス稼働が始まったら全LEDを点灯
    LED_BLUE = 1;
    LED_GREEN = 1;

    // ２秒経過したら全LEDを消灯
    wait(2);
    LED_BLUE = 0;
    LED_GREEN = 0;

    printf("----- U2F Authenticator start -----\r\n");
}

//
// 処理ステータスに対応する処理
//
void u2f_process_state_main(void)
{
    switch (u2f_process_state) {
    case U2FPS_RECV_REQ:
        // LEDを点灯
        LED_GREEN = 1;
        printf("Received message from U2F client\r\n");
        u2f_process_state = U2FPS_RECV_REQ_DONE;
        break;
    case U2FPS_XFER_REQ:
        // LEDを点灯し、タイマーをスタート
        LED_BLUE = 1;
        u2f_process_state_start_timer();
        printf("Sent message to U2F Helper\r\n");
        u2f_process_state = U2FPS_XFER_REQ_DONE;
        break;
    case U2FPS_HELPER_TIMEOUT:
        // エラーレスポンスデータを送信
        // 0x7f = ERROR_OTHER
        send_error_response_packet(0x7f);
        printf("U2F Helper timed out\r\n");
        // LEDを消灯
        LED_BLUE = 0;
        LED_GREEN = 0;
        u2f_process_state = U2FPS_NONE;
        break;
    case U2FPS_XFER_RES:
        // LEDを消灯
        LED_GREEN = 0;
        u2f_process_state = U2FPS_NONE;
        printf("Sent message to U2F client\r\n");
        break;
    default:
        break;
    }
}

void u2f_process_state_on_receive_response(void)
{
    // LEDを消灯し、タイマーをキャンセル
    LED_BLUE = 0;
    u2f_process_state_stop_timer();
    printf("Received message from U2F Helper\r\n");
}
