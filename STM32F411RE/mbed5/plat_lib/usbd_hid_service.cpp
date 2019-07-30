/* 
 * File:   usbd_hid_service.cpp
 * Author: makmorit
 *
 * Created on 2019/07/29, 14:30
 */
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "USBFIDO.h"

#include "fido_hid_receive.h"
#include "fido_hid_send.h"

USBFIDO *usbFIDO;

static bool m_report_received;
static bool m_report_sent;

void usbd_hid_init(void)
{
    //
    // USB HIDデバイスを初期化
    //  PC-->mbed: 64バイト
    //  mbed-->PC: 64バイト
    //
    usbFIDO = new USBFIDO(64, 64);

    // フラグをクリア
    m_report_received = false;
    m_report_sent = false;
}

void usbd_hid_do_process(void)
{
    // HIDデータフレームを受信
    if (usbFIDO->read(&usbFIDO->recv_report)) {
        // Output reportから受信フレームを取得し、
        // request_frame_bufferに格納
        // 受信フレーム数は、request_frame_numberに設定される
        m_report_received = fido_hid_receive_request_frame(
            usbFIDO->recv_report.data, usbFIDO->recv_report.length);
    }

    if (m_report_received) {
        // 受信データが揃った場合
        // FIDO USB HIDサービスを実行
        m_report_received = false;
        fido_hid_receive_on_request_received();
    }

    if (m_report_sent) {
        // 後続のデータがあれば送信
        m_report_sent = false;
        fido_hid_send_input_report_complete();
    }    
}

//
// C --> CPP 呼出用インターフェース
//
void _usbd_hid_frame_send(uint8_t *buffer_for_send, size_t size)
{
    // 受領メッセージを送信メッセージ領域にコピー
    memcpy(usbFIDO->send_report.data, buffer_for_send, size);

    // HIDデータフレームを送信
    if (usbFIDO->send(&usbFIDO->send_report)) {
        m_report_sent = true;
    }
}
