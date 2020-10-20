/* 
 * File:   demo_ble_peripheral_auth.c
 * Author: makmorit
 *
 * Created on 2019/10/22, 11:48
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// プラットフォーム固有のインターフェース
#include "usbd_service.h"
#include "ble_service_central.h"
#include "ble_service_central_stat.h"
#include "ble_service_common.h"

// 業務処理インターフェース
#include "fido_command.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// デモ機能インターフェース
#include "demo_ble_peripheral_auth.h"

// for debug log and hexdump
#define LOG_HEXDUMP_DEBUG_ADVDATA   false
#define LOG_HEXDUMP_DEBUG_SCANPARAM false

// コマンド文字列
#define SET_AUTH_UUID_COMMAND           "set_auth_uuid"
#define RESET_AUTH_UUID_COMMAND         "reset_auth_uuid"
#define SET_AUTH_UUID_SCAN_SEC_COMMAND  "set_auth_uuid_scan_sec"

// 各種定数
#define UUID_STRING_LEN  36
#define SCAN_SEC_DEFAULT 3
#define SCAN_ENABLE_DEFAULT 0
#define NEED_PAIRING_DEFAULT 0
#define PEER_ADDR_SIZE 6

// スキャン対象サービスUUID（文字列形式）、スキャン秒数、自動認証有効化フラグ、ペアリング要否フラグを保持
static char    service_uuid_string[UUID_STRING_LEN+1];
static uint8_t service_uuid_scan_sec = SCAN_SEC_DEFAULT;
static uint8_t service_uuid_scan_enable = SCAN_ENABLE_DEFAULT;
static uint8_t service_uuid_need_pairing = NEED_PAIRING_DEFAULT;

// キーハンドル／クレデンシャルIDに格納される
// BLEスキャン用パラメーター
//   0 - 15: サービスUUID（16バイト）
//  16 - 21: Bluetoothアドレス（6バイト）
static uint8_t scan_param_bytes[32];
static size_t  scan_param_bytes_size;

// 関数プロトタイプ
static bool register_or_match_scan_param(bool is_register, uint8_t *uuid_bytes, size_t uuid_bytes_size, uint8_t *connected_address);
static bool demo_ble_peripheral_auth_start_second_scan(uint8_t *p_scan_param);

static void parse_scan_param_flags(uint32_t *param_service_uuid_scan_enable)
{
    // uint32_t 形式で１ワード内に同梱されている
    // 自動認証有効化フラグ、ペアリング要否フラグを、
    // 登録ワードの先頭バイトから順に抽出
    uint8_t *p_param = (uint8_t *)param_service_uuid_scan_enable;
    service_uuid_scan_enable = p_param[0];
    service_uuid_need_pairing = p_param[1];
}

static void generate_scan_param_flags(uint32_t *param_service_uuid_scan_enable)
{
    // 自動認証有効化フラグ、ペアリング要否フラグを、
    // 先頭から順に uint32_t 形式で１ワード内に同梱
    uint8_t param_array[] = {
        service_uuid_scan_enable, 
        service_uuid_need_pairing, 
        0x00, 0x00};
    memcpy(param_service_uuid_scan_enable, param_array, sizeof(param_array));
}

static void init_auth_param(void)
{
    // 初期値を設定
    memset(service_uuid_string, 0, sizeof(service_uuid_string));
    service_uuid_scan_sec = SCAN_SEC_DEFAULT;
    service_uuid_scan_enable = SCAN_ENABLE_DEFAULT;
    service_uuid_need_pairing = NEED_PAIRING_DEFAULT;
}

static void save_auth_param(void)
{
    uint8_t *p_uuid_string = (uint8_t *)service_uuid_string;
    uint32_t scan_sec      = (uint32_t)service_uuid_scan_sec;

    // 自動認証有効化フラグ、ペアリング要否フラグ＝登録ワードの先頭バイトから順に設定
    uint32_t scan_enable;
    generate_scan_param_flags(&scan_enable);

    if (fido_flash_blp_auth_param_write(p_uuid_string, scan_sec, scan_enable) == false) {
        fido_log_error("Failed to save BLE peripheral auth parameter to flash ROM");
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
        for (int i = 0; i < UUID_STRING_LEN; i++) {
            int c = toupper(service_uuid_string[i]);
            service_uuid_string[i] = (uint8_t)c;
        }
    }

    uint8_t scan_sec = (uint8_t)fido_flash_blp_auth_param_service_uuid_scan_sec();
    if (scan_sec != 0) {
        service_uuid_scan_sec = scan_sec;
    }

    // 自動認証有効化フラグ、ペアリング要否フラグ＝登録ワードの先頭バイトから順に抽出
    uint32_t param_service_uuid_scan_enable = fido_flash_blp_auth_param_service_uuid_scan_enable();
    parse_scan_param_flags(&param_service_uuid_scan_enable);
}

static void clear_scan_parameter(void)
{
    // BLEスキャンパラメーターをクリア
    scan_param_bytes_size = 0;
    memset(scan_param_bytes, 0, sizeof(scan_param_bytes));
}

void demo_ble_peripheral_auth_param_init(void)
{
    // 初期値を設定
    init_auth_param();

    // Flash ROMに設定されている場合は読み出す
    restore_auth_param();
    
    // BLEスキャンパラメーターを事前にクリア
    clear_scan_parameter();
}

size_t demo_ble_peripheral_auth_scan_param_prepare(uint8_t *p_buff)
{
    // BLE自動認証機能用のスキャンパラメーターを、
    // 引数で指定された領域に格納
    // 先頭にパラメーター長を設定
    p_buff[0] = (uint8_t)scan_param_bytes_size;
    size_t offset = 1;

    if (scan_param_bytes_size > 0) {
        // スキャンが成功している場合
        // 後続バイトに、スキャンパラメーターのバイト配列を格納
        memcpy(p_buff + offset, scan_param_bytes, scan_param_bytes_size);
        offset += scan_param_bytes_size;
    }

    // パラメーター領域の長さを戻す
    return offset;
}

static void scan_parameter_buffer_set(uint8_t *uuid_bytes, size_t uuid_bytes_size, uint8_t *addr)
{
    // 領域の初期化
    size_t offset = 0;
    clear_scan_parameter();

    if (uuid_bytes == NULL || uuid_bytes_size == 0) {
        // スキャン結果が指定されていない場合は終了
        return;
    }

    // ログイン処理時に必要なBLEスキャンパラメーターを、
    // キーハンドル／クレデンシャルID生成用の一時バッファに保持
    //   スキャンされたBLEデバイスのUUID
    memcpy(scan_param_bytes, uuid_bytes, uuid_bytes_size);
    offset += uuid_bytes_size;
    //   Bluetoothアドレス
    memcpy(scan_param_bytes + offset, addr, PEER_ADDR_SIZE);
    offset += PEER_ADDR_SIZE;
    //   パラメーター長
    scan_param_bytes_size = offset;

#if LOG_HEXDUMP_DEBUG_SCANPARAM
    fido_log_debug("BLE peripheral device scan parameter byte (%d bytes):", scan_param_bytes_size);
    fido_log_print_hexdump_debug(scan_param_bytes, scan_param_bytes_size);
#endif
}

static void resume_function_after_scan(bool is_register)
{
#if LOG_HEXDUMP_DEBUG_ADVDATA
    // 統計情報をデバッグ出力
    ble_service_central_stat_debug_print();
#endif

    // スキャン対象サービスUUIDが、スキャン統計情報に含まれているかどうかチェック
    ADV_STAT_INFO_T *info = ble_service_central_stat_match_uuid(service_uuid_string);
    if (info == NULL) {
        // 見つからなかった時の処理
        fido_log_debug("BLE peripheral device (for FIDO %s) not scanned.", is_register ? "register" : "authenticate");
        fido_user_presence_verify_on_ble_scan_end(false);

    } else {
        // 見つかった時の処理
        // 複数スキャンされた場合は、最もRSSI値が大きいBLEデバイスが戻ります。
        fido_log_debug("BLE peripheral device (for FIDO %s) scanned (NAME=%s, ADDR=%s)", 
            is_register ? "register" : "authenticate",
            info->dev_name, ble_service_central_stat_btaddr_string(info->peer_addr.addr));

        // 後続の処理を実行
        bool found = register_or_match_scan_param(is_register, info->uuid_bytes, info->uuid_bytes_size, info->peer_addr.addr);
        fido_user_presence_verify_on_ble_scan_end(found);
    }
}

static bool register_or_match_scan_param(bool is_register, uint8_t *uuid_bytes, size_t uuid_bytes_size, uint8_t *connected_address)
{
    bool found = true;
    if (is_register) {
        // Registerの場合は、BLEスキャンパラメーターを登録するため
        // 統計情報とBluetoothアドレスをバッファに保持
        scan_parameter_buffer_set(uuid_bytes, uuid_bytes_size, connected_address);

    } else {
        // Authenticateの場合は、BLEスキャンパラメーターに登録された
        // Bluetoothアドレスとマッチングを行う
        if (ble_service_central_stat_match_scan_param(scan_param_bytes, uuid_bytes, uuid_bytes_size, connected_address) == false) {
            // 失敗した場合は以降の処理を中止
            found = false;
        }
    }

    // マッチング結果を戻す
    fido_log_debug("BLE peripheral device (for FIDO %s) %s (Bluetooth address=%s)", 
        is_register ? "register" : "authenticate",
        found ? "found" : "not found",
        ble_service_central_stat_btaddr_string(connected_address));
    return found;
}

bool demo_ble_peripheral_auth_scan_enable(void)
{
    if (ble_service_peripheral_mode()) {
        // BLEペリフェラルモードである場合は
        // 利用不可なので、falseを戻す
        return false;
    }

    if (service_uuid_string[0] == 0 ||
        service_uuid_scan_enable == SCAN_ENABLE_DEFAULT) {
        // スキャン対象サービスUUIDが指定されていない場合、
        // または自動認証有効化フラグが設定されていない場合は、
        // 利用不可なので、falseを戻す
        return false;
    }

    return true;
}

bool demo_ble_peripheral_auth_start_scan(void *context)
{
    if (context != NULL) {
        // ログイン処理の場合は、
        // BLEスキャンパラメーターを引き渡す
        return demo_ble_peripheral_auth_start_second_scan((uint8_t *)context);
    }
    
    // BLEスキャンパラメーターを事前にクリア
    clear_scan_parameter();

    if (demo_ble_peripheral_auth_scan_enable() == false) {
        // BLE自動認証が利用できない場合は false を戻し終了
        return false;
    }

    // 指定したサービスUUIDを使用し、
    // 指定秒数間スキャンを実行
    ble_service_central_scan_start(service_uuid_scan_sec * 1000, resume_function_after_scan, true);
    return true;
}

static bool demo_ble_peripheral_auth_start_second_scan(uint8_t *p_scan_param)
{
    // BLEスキャンパラメーターが設定されていない場合は終了
    size_t param_size = (size_t)p_scan_param[0];
    if (param_size == 0) {
        return false;
    }
    
    // BLEスキャンパラメーターを保持
    // （バイト長を保持している先頭１バイトは不要）
    memcpy(scan_param_bytes, p_scan_param + 1, param_size);

    // 指定したサービスUUIDを使用し、指定秒数間スキャンを実行
    ble_service_central_scan_start(service_uuid_scan_sec * 1000, resume_function_after_scan, false);
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
    // (1) 自動認証有効化フラグ、ペアリング要否フラグ
    char *s = (char *)(request + 1);
    char *p = strtok(s, ",");
    if (p == NULL) {
        return;
    }
    // 登録ワードの先頭バイトから順に抽出
    uint32_t param_service_uuid_scan_enable = (uint32_t)atoi(p);
    parse_scan_param_flags(&param_service_uuid_scan_enable);

    // (2) スキャン対象サービスUUID
    // (3) スキャン秒数
    char *p1 = strtok(NULL, ",");
    char *p2 = strtok(NULL, ",");
    if (p2 == NULL) {
        // スキャン対象サービスUUIDがブランクの場合
        // (設定されない場合、strtok は３つの項目を戻さないための判定)
        memset(service_uuid_string, 0, sizeof(service_uuid_string));
        service_uuid_scan_sec = (uint8_t)atoi(p1);

    } else {
        memcpy(service_uuid_string, p1, strlen(p1));
        service_uuid_scan_sec = (uint8_t)atoi(p2);
    }
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
            init_auth_param();
            // パラメーターをFlash ROMに保存
            save_auth_param();
            break;
        default:
            break;
    }
}

bool demo_ble_peripheral_auth_param_response(uint8_t cmd_type, uint8_t *response, size_t *response_size)
{
    // 自動認証有効化フラグ、ペアリング要否フラグ
    // ＝登録ワードの先頭バイトから順に１ワード内に設定
    uint32_t param_service_uuid_scan_enable;
    generate_scan_param_flags(&param_service_uuid_scan_enable);

    // 領域を初期化
    memset(response, 0x00, *response_size);
    //
    // CSVを編集
    //   <自動認証有効化フラグ>,<スキャン対象サービスUUID>,<スキャン秒数>
    //  （解除時は、UUIDに、長さ０の文字列を設定）
    //
    sprintf((char *)response, "%ld,%s,%d", 
        param_service_uuid_scan_enable, service_uuid_string, service_uuid_scan_sec);
    *response_size = strlen((char *)response);
    return true;
}
