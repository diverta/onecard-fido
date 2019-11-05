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
#include "ble_service_central.h"
#include "ble_service_central_stat.h"
#include "ble_service_peripheral.h"

// 業務処理インターフェース
#include "fido_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// デモ機能インターフェース
#include "demo_cdc_service.h"
#include "demo_ble_peripheral_auth.h"

// for debug log and hexdump
#define LOG_HEXDUMP_DEBUG_ADVDATA   false

// コマンド文字列
#define SET_AUTH_UUID_COMMAND           "set_auth_uuid"
#define RESET_AUTH_UUID_COMMAND         "reset_auth_uuid"
#define SET_AUTH_UUID_SCAN_SEC_COMMAND  "set_auth_uuid_scan_sec"

// 各種定数
#define UUID_STRING_LEN  36
#define SCAN_SEC_DEFAULT 3
#define SCAN_ENABLE_DEFAULT 0
#define CDC_OK_RESPONSE  "OK\r\n"

// スキャン対象サービスUUID（文字列形式）、スキャン秒数、自動認証有効化フラグを保持
static char    service_uuid_string[UUID_STRING_LEN+1];
static uint8_t service_uuid_scan_sec = SCAN_SEC_DEFAULT;
static uint8_t service_uuid_scan_enable = SCAN_ENABLE_DEFAULT;

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
    service_uuid_scan_enable = SCAN_ENABLE_DEFAULT;

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

static void resume_function_after_scan(void)
{
#if LOG_HEXDUMP_DEBUG_ADVDATA
    // 統計情報をデバッグ出力
    ble_service_central_stat_debug_print();
#endif

    // スキャン対象サービスUUIDが、スキャン統計情報に含まれているかどうかチェック
    ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(service_uuid_string);
    if (info == NULL) {
        // 見つからなかった時の処理
        fido_log_debug("BLE peripheral device (for FIDO authenticate) not found.");
        fido_user_presence_verify_on_ble_scan_end(false);

    } else {
        // 見つかった時の処理
        // 複数スキャンされた場合は、最もRSSI値が大きいBLEデバイスが戻ります。
        fido_log_debug("BLE peripheral device (for FIDO authenticate) found (NAME=%s, ADDR=%s)", 
            info->dev_name, ble_service_central_stat_btaddr_string(info->peer_addr));
        fido_user_presence_verify_on_ble_scan_end(true);
    }
}

bool demo_ble_peripheral_auth_start_scan(void)
{
    if (ble_service_peripheral_mode()) {
        // BLEペリフェラルモードである場合は
        // 利用不可なので、falseを戻す
        return false;
    }

    demo_ble_peripheral_auth_param_init();
    if (service_uuid_string[0] == 0 ||
        service_uuid_scan_enable == SCAN_ENABLE_DEFAULT) {
        // スキャン対象サービスUUIDが指定されていない場合、
        // または自動認証有効化フラグが設定されていない場合は、
        // 利用不可なので、falseを戻す
        return false;
    }

    // 指定したサービスUUIDを使用し、
    // 指定秒数間スキャンを実行
    ble_service_central_scan_start(service_uuid_scan_sec * 1000, resume_function_after_scan);
    return true;
}

//
// 管理ツールから、USB HID経由で実行される設定コマンド
//
void parse_auth_param_request(uint8_t *request, size_t request_size)
{
    // Flash ROMに設定されている値を読み出す
    // 未設定の場合は初期値が設定されます
    demo_ble_peripheral_auth_param_init();

    // CSV各項目を分解
    // CSVは、リクエストの２バイト目以降
    // (1) 自動認証有効化フラグ
    char *s = (char *)(request + 1);
    char *p = strtok(s, ",");
    if (p == NULL) {
        return;
    }
    service_uuid_scan_enable = (uint8_t)atoi(p);
    // (2) スキャン対象サービスUUID
    p = strtok(NULL, ",");
    if (p == NULL) {
        return;
    }
    memcpy(service_uuid_string, p, UUID_STRING_LEN);
    // (3) スキャン秒数
    p = strtok(NULL, ",");
    if (p == NULL) {
        return;
    }
    service_uuid_scan_sec = (uint8_t)atoi(p);
}

void demo_ble_peripheral_auth_param_request(uint8_t *request, size_t request_size)
{
    fido_log_info("BLE peripheral auth param request:");
    fido_log_print_hexdump_debug(request, request_size);

    // データの１バイト目からコマンド種別を取得
    switch (request[0]) {
        case 1:
            // 読込の場合
            // Flash ROMに設定されている値を読み出す
            // 未設定の場合は初期値が設定されます
            demo_ble_peripheral_auth_param_init();
            break;
        case 2:
            // 書出の場合
            // CSVを各項目に分解し、内部変数に設定
            parse_auth_param_request(request, request_size);
            // パラメーターをFlash ROMに保存
            save_auth_param();
            break;
        case 3:
            // 解除の場合
            // 初期値を設定
            memset(service_uuid_string, 0, sizeof(service_uuid_string));
            service_uuid_scan_sec = SCAN_SEC_DEFAULT;
            service_uuid_scan_enable = SCAN_ENABLE_DEFAULT;
            // パラメーターをFlash ROMに保存
            save_auth_param();
            break;
        default:
            break;
    }
}

bool demo_ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size)
{
    // 領域を初期化
    memset(response, 0x00, *response_size);
    //
    // CSVを編集
    //   <自動認証有効化フラグ>,<スキャン対象サービスUUID>,<スキャン秒数>
    //  （解除時は、UUIDに、長さ０の文字列を設定）
    //
    sprintf((char *)response, "%d,%s,%d", 
        service_uuid_scan_enable, service_uuid_string, service_uuid_scan_sec);
    *response_size = strlen((char *)response);
    return true;
}
