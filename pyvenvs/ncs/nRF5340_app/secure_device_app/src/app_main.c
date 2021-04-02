/* 
 * File:   app_main.c
 * Author: makmorit
 *
 * Created on 2021/04/02, 15:04
 */
#include <zephyr/types.h>
#include "os_mgmt/os_mgmt.h"
#include "img_mgmt/img_mgmt.h"

#include "app_main.h"

//
// 業務処理エントリー関数
//
void app_main(void) 
{
    os_mgmt_register_group();
    img_mgmt_register_group();
    app_bluetooth_start();
}
