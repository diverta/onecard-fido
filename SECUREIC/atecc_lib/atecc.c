/* 
 * File:   atecc.c
 * Author: makmorit
 *
 * Created on 2020/08/12, 11:19
 */
//
// プラットフォーム非依存コード
//
#include "atecc.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// ATECCx08A関連
//
#include "atecc_command.h"
#include "atecc_iface.h"
#include "atecc_read.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_CONFIG true

// 設定情報を保持
static uint8_t ateccx08a_config_bytes[ATECC_CONFIG_SIZE];

// 初期化処理実行済みフラグ
static bool atecc_init_done = false;

// シリアルナンバーを保持
static uint8_t atecc_serial_num[ATECC_SERIAL_NUM_SIZE];

// シリアルナンバーを表示可能なHex文字列形式にするためのバッファ
static char serial_num_str[20];

static bool get_atecc_serial_num(void)
{
    ATECC_STATUS status = atecc_read_serial_number(atecc_serial_num);
    if (status != ATECC_SUCCESS) {
        fido_log_error("get_atecc_serial_num failed: atecc_read_serial_number() returns 0x%02x", status);
        return false;
    }
#if LOG_HEXDUMP_DEBUG_CONFIG
    fido_log_debug("Serial number:");
    fido_log_print_hexdump_debug(atecc_serial_num, ATECC_SERIAL_NUM_SIZE);
#endif
    return true;
}

char *atecc_get_serial_num_str(void)
{
    // ATECC608Aの初期化
    memset(serial_num_str, 0, sizeof(serial_num_str));
    if (atecc_initialize() == false) {
        return serial_num_str;
    }

    // ATECC608Aのシリアルナンバーを、表示可能なHex文字列形式で戻す
    //   例：0123d560b2d9470dee（18バイト）
    for (int i = 0; i < ATECC_SERIAL_NUM_SIZE; i++) {
        char *serial_num_str_ = serial_num_str;
        sprintf(serial_num_str, "%s%02X", serial_num_str_, atecc_serial_num[i]);
    }

    // デバイスを解放
    atecc_finalize();
    return serial_num_str;
}

bool atecc_initialize(void)
{
    if (atecc_init_done) {
        // 初期化処理が実行済みの場合は終了
        return true;
    }
    fido_log_info("atecc_initialize start");

    // デバイスの初期化
    ATECC_STATUS status = atecc_init();
    if (status != ATECC_SUCCESS) {
        fido_log_error("atecc_initialize failed: atecc_init() returns 0x%02x", status);
        return false;
    }

    // シリアル番号を取得
    if (get_atecc_serial_num() == false) {
        return false;
    }

    // Configがデフォルトのままであれば変更を行う
    if (atecc_setup_config() == false) {
        return false;
    }

    // 初期化処理は実行済み
    atecc_init_done = true;
    fido_log_info("atecc_initialize success");
    return true;
}

void atecc_finalize(void)
{
    if (atecc_init_done) {
        // デバイスを解放
        atecc_release();
        atecc_init_done = false;
        fido_log_info("atecc_finalize done");
    }
}

bool atecc_get_config_bytes(void)
{
    ATECC_STATUS status = atecc_read_config_zone(ateccx08a_config_bytes);
    if (status != ATECC_SUCCESS) {
        fido_log_error("atecc_get_config_bytes failed: atecc_read_config_zone() returns 0x%02x", status);
        return false;
    }
#if LOG_HEXDUMP_DEBUG_CONFIG
    fido_log_debug("Config zone data (128 bytes):");
    fido_log_print_hexdump_debug(ateccx08a_config_bytes,      64);
    fido_log_print_hexdump_debug(ateccx08a_config_bytes + 64, 64);
#endif
    return true;
}
