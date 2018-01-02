//
//  main.m
//  SampleTransceiver
//
//  Created by makmorit on 2018/01/02.
//  Copyright © 2018 Diverta Inc. All rights reserved.
//

#import <Foundation/Foundation.h>

#include <stdio.h>
#include <stdlib.h>

#define LINE_BUF_SIZE  1024

int main_process(void) {
    static char line[LINE_BUF_SIZE];
    static char size_buf[4];
    char *received = NULL;
    
    do {
        // 標準入力から、最初の4バイトを読み込み、
        // Chromeエクステンションから送信されたデータのバイト数を計算
        size_t s = fread(size_buf, sizeof(char), sizeof(size_buf), stdin);
        if (s < 1) {
            NSLog(@"fread failed");
            break;
        }
        int size = (int)(size_buf[1] * 256 + size_buf[0]);
        
        // 続いてデータ部を取得
        received = fgets(line, size + 1, stdin);
        if (received == NULL) {
            NSLog(@"fgets failed");
            break;
        }
        NSLog(@"Received from chrome extension [%s]", received);

        // for research
        // 受信したデータ部をそのままChromeエクステンションへ送信
        s = fwrite(size_buf, sizeof(char), sizeof(size_buf), stdout);
        if (s < 1) {
            NSLog(@"fwrite failed");
            break;
        }
        fprintf(stdout, "%s", received);
        fflush(stdout);

    } while (received != NULL);

    NSLog(@"Sub process terminated");
    return 0;
}


int main(int argc, const char * argv[]) {
    @autoreleasepool {
        main_process();
    }
    return 0;
}
