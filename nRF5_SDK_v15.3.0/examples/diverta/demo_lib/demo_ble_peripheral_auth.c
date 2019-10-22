/* 
 * File:   demo_ble_peripheral_auth.c
 * Author: makmorit
 *
 * Created on 2019/10/22, 11:48
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// プラットフォーム固有のインターフェース
#include "usbd_service.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// デモ機能インターフェース
#include "demo_cdc_service.h"
#include "demo_ble_peripheral_auth.h"

// コマンド文字列
#define SET_AUTH_UUID_COMMAND           "set_auth_uuid"
#define RESET_AUTH_UUID_COMMAND         "reset_auth_uuid"
#define SET_AUTH_UUID_SCAN_SEC_COMMAND  "set_auth_uuid_scan_sec"

// 各種定数
#define UUID_STRING_LEN  36
#define SCAN_SEC_DEFAULT 3
#define CDC_OK_RESPONSE  "OK\r\n"

// スキャン対象サービスUUID（文字列形式）、スキャン秒数を保持
static char    service_uuid_string[UUID_STRING_LEN+1];
static uint8_t service_uuid_scan_sec = SCAN_SEC_DEFAULT;

static void save_auth_param(void)
{
    uint8_t *p_uuid_string = (uint8_t *)service_uuid_string;
    uint32_t scan_sec      = (uint32_t)service_uuid_scan_sec;

    if (fido_flash_blp_auth_param_write(p_uuid_string, scan_sec) == false) {
        demo_cdc_send_response_buffer_set("Failed to save parameter to flash ROM.\r\n");
    }
}

static void restore_auth_param(void)
{
    if (fido_flash_blp_auth_param_read() == false) {
        return;
    }

    char *p_uuid_string = (char *)fido_flash_blp_auth_param_service_uuid_string();
    if (p_uuid_string[0] != 0) {
        memcpy(service_uuid_string, p_uuid_string, UUID_STRING_LEN);
        service_uuid_string[UUID_STRING_LEN] = 0;
    }

    uint8_t scan_sec = (uint8_t)fido_flash_blp_auth_param_service_uuid_scan_sec();
    if (scan_sec != 0) {
        service_uuid_scan_sec = scan_sec;
    }
}

static bool set_auth_uuid(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    // デモ機能用のコマンドを解析して実行
    size_t command_len = strlen(SET_AUTH_UUID_COMMAND);
    if (strncmp(p_cdc_buffer, SET_AUTH_UUID_COMMAND, command_len) != 0) {
        return false;
    }

    uint8_t len = command_len + 1;
    if (strlen(p_cdc_buffer) == (len + UUID_STRING_LEN)) {
        // パラメーターが入力されている場合は退避し、Flash ROMに保存
        //   UUID文字列＝36文字分
        memcpy(service_uuid_string, p_cdc_buffer + len, UUID_STRING_LEN);
        save_auth_param();
        demo_cdc_send_response_buffer_set(CDC_OK_RESPONSE);

    } else {
        // パラメーター入力エラー
        demo_cdc_send_response_buffer_set("Invalid UUID parameter.\r\n");
    }

    return true;
}

static bool reset_auth_uuid(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    if (strcmp(p_cdc_buffer, RESET_AUTH_UUID_COMMAND) != 0) {
        return false;
    }

    // 格納領域を初期化し、パラメーターを保存
    memset(service_uuid_string, 0, sizeof(service_uuid_string));
    save_auth_param();
    demo_cdc_send_response_buffer_set(CDC_OK_RESPONSE);
    return true;
}

static bool set_auth_uuid_scan_sec(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    // デモ機能用のコマンドを解析して実行
    size_t command_len = strlen(SET_AUTH_UUID_SCAN_SEC_COMMAND);
    if (strncmp(p_cdc_buffer, SET_AUTH_UUID_SCAN_SEC_COMMAND, command_len) != 0) {
        return false;
    }
    
    // パラメーターを解析
    if (strlen(p_cdc_buffer) == command_len) {
        // 引数がない場合はデフォルトにリセットし、
        // パラメーターをFlash ROMに保存
        service_uuid_scan_sec = SCAN_SEC_DEFAULT;
        save_auth_param();
        demo_cdc_send_response_buffer_set(CDC_OK_RESPONSE);

    } else {
        uint8_t sec = (uint32_t)atoi(p_cdc_buffer + command_len);
        if (sec < 1 || sec > 9) {
            // エラーメッセージを表示する
            demo_cdc_send_response_buffer_set("Parameter must be in the range 1 to 9 (sec).\r\n");

        } else {
            // パラメーターをFlash ROMに保存
            service_uuid_scan_sec = sec;
            save_auth_param();
            demo_cdc_send_response_buffer_set(CDC_OK_RESPONSE);
        }        
    }

    return true;
}

static bool display_param(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    if (strcmp(p_cdc_buffer, "auth_uuid") == 0) {
        if (strlen(service_uuid_string) == 0) {
            demo_cdc_send_response_buffer_set("Service UUID for scan: not specified\r\n");
        } else {
            demo_cdc_send_response_buffer_set("Service UUID for scan: %s\r\n", service_uuid_string);
        }
        return true;
    }
    if (strcmp(p_cdc_buffer, "auth_uuid_scan_sec") == 0) {
        demo_cdc_send_response_buffer_set("Service UUID scanning time: %d sec\r\n", service_uuid_scan_sec);
        return true;
    }

    return false;
}

void demo_ble_peripheral_auth_param_init(void)
{
    // 初期値を設定
    memset(service_uuid_string, 0, sizeof(service_uuid_string));
    service_uuid_scan_sec = SCAN_SEC_DEFAULT;

    // Flash ROMに設定されている場合は読み出す
    restore_auth_param();
}

bool demo_ble_peripheral_auth_param_set(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    if (set_auth_uuid_scan_sec(p_cdc_buffer, cdc_buffer_size)) {
        return true;
    }
    if (set_auth_uuid(p_cdc_buffer, cdc_buffer_size)) {
        return true;
    }
    if (reset_auth_uuid(p_cdc_buffer, cdc_buffer_size)) {
        return true;
    }
    if (display_param(p_cdc_buffer, cdc_buffer_size)) {
        return true;
    }
    return false;
}
