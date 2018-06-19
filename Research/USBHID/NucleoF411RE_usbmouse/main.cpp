#include <stdio.h>

#include "mbed.h"
#include "USBU2FAuthenticator.h"

USBU2FAuthenticator u2fAuthenticator(true);

int main(void) {
    wait(1);
    printf("----- U2F Authenticator sample start -----\r\n");
    
    //
    // １バイトのメッセージを出力
    //
    HID_REPORT report;
    report.data[0] = 0x01;
    report.length = 1;
    bool ret = u2fAuthenticator.send(&report);
    printf("u2fAuthenticator.send done (return=%s)\r\n", ret? "true" : "false");

    printf("----- U2F Authenticator sample end -----\r\n");
    
    while (true);
}