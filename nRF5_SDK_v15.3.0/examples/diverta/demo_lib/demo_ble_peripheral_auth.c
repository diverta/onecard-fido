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

#define UUID_STRING_LEN  36
#define SCAN_SEC_DEFAULT 3

// スキャン対象サービスUUID（文字列形式）、スキャン秒数を保持
static char    service_uuid_string[UUID_STRING_LEN+1];
static uint8_t service_uuid_scan_sec = SCAN_SEC_DEFAULT;

static bool set_auth_uuid(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    // デモ機能用のコマンドを解析して実行
    size_t command_len = strlen(SET_AUTH_UUID_COMMAND);
    if (strncmp(p_cdc_buffer, SET_AUTH_UUID_COMMAND, command_len) != 0) {
        return false;
    }

    uint8_t len = command_len + 1;
    if (strlen(p_cdc_buffer) == (len + UUID_STRING_LEN)) {
        // パラメーターが入力されている場合は退避
        //   UUID文字列＝36文字分
        memcpy(service_uuid_string, p_cdc_buffer + len, UUID_STRING_LEN);
        fido_log_debug("UUID parameter: %s", service_uuid_string);

    } else {
        // パラメーター入力エラー
        fido_log_debug("Invalid UUID parameter.");
    }

    return true;
}

static bool reset_auth_uuid(char *p_cdc_buffer, size_t cdc_buffer_size)
{
    if (strcmp(p_cdc_buffer, RESET_AUTH_UUID_COMMAND) != 0) {
        return false;
    }

    // 格納領域を初期化
    memset(service_uuid_string, 0, sizeof(service_uuid_string));
    fido_log_debug("Reset UUID parameter.");
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
    service_uuid_scan_sec = (uint32_t)atoi(p_cdc_buffer + command_len);
    if (strlen(p_cdc_buffer) == command_len) {
        // 引数がない場合はデフォルトにリセット
        service_uuid_scan_sec = SCAN_SEC_DEFAULT;
        fido_log_debug("default interval(%d)", service_uuid_scan_sec);

    } else if (service_uuid_scan_sec < 1 || service_uuid_scan_sec > 9) {
        // エラーメッセージを表示する
        fido_log_debug("Parameter must be in the range 1 to 9 (sec).", service_uuid_scan_sec);
        //sprintf(cdc_response_buff, "Parameter must be in the range 1 to 9 (sec).\r\n");
        //cdc_response_send = true;

    } else {
        fido_log_debug("changed interval(%d)", service_uuid_scan_sec);
    }

    return true;
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
    return false;
}

