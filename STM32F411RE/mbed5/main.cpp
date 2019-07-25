#include "mbed.h"
#include "USBDHIDService.h"

int main(void)
{
    //
    // PC-->mbed: 64バイト
    // mbed-->PC: 64バイト
    //
    printf("main start\r\n");
    USBDHIDService hid(64, 64);

    // 初期化処理
    hid.doInitialize();

    // 主処理
    while (hid.doProcess());
    printf("main end\r\n");
}
