#include "mbed.h"
#include "USBHID.h"

#include "USBDHIDService.h"

//
// PC-->mbed: 64バイト
// mbed-->PC: 64バイト
USBDHIDService hid(64, 64);

// This report will contain data to be sent
HID_REPORT send_report;
HID_REPORT recv_report;
 
int main(void)
{
    printf("Start\r\n");
    send_report.length = 64;

    while (1) {
        // HIDデータフレームを受信
        if (hid.read(&recv_report)) {
            printf("recv:");
            for (int i = 0; i < recv_report.length; i++) {
                if (i % 16 == 0) {
                    printf("\r\n");
                }
                printf("%02x ", recv_report.data[i]);

                // 受信メッセージを送信メッセージ領域にコピー
                send_report.data[i] = recv_report.data[i];
            }
            printf("\r\n");

            // HIDデータフレームを送信
            // 受信メッセージをecho back
            hid.sendNB(&send_report);
            printf("sent: done.\r\n");
        }

        wait(0.1);
    }
}
