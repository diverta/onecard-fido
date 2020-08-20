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
#include "atecc_priv.h"
#include "atecc_read.h"
#include "atecc_util.h"
#include "atecc_write.h"

// for debug hex dump data
#define LOG_HEXDUMP_DEBUG_CONFIG false

// データ編集用エリア（領域節約のため共通化）
static uint8_t work_buf_1[256];
static uint8_t work_buf_2[32];

// 初期化処理実行済みフラグ
static bool atecc_init_done = false;

// シリアルナンバーを保持
static uint8_t atecc_serial_num[ATECC_SERIAL_NUM_SIZE];

static bool get_atecc_serial_num(void)
{
    if (atecc_read_serial_number(atecc_serial_num) == false) {
        fido_log_error("get_atecc_serial_num failed");
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
    char *serial_num_str = (char *)work_buf_1;
    memset(serial_num_str, 0, sizeof(serial_num_str));
    if (atecc_is_available() == false) {
        return serial_num_str;
    }

    // ATECC608Aのシリアルナンバーを、表示可能なHex文字列形式で戻す
    //   例：0123d560b2d9470dee（18バイト）
    for (int i = 0; i < ATECC_SERIAL_NUM_SIZE; i++) {
        char *serial_num_str_ = serial_num_str;
        sprintf(serial_num_str, "%s%02X", serial_num_str_, atecc_serial_num[i]);
    }

    return serial_num_str;
}

bool atecc_is_available(void)
{
    // ATECC608Aに接続し、
    // 初期化処理が実行済みの場合は true を戻す
    return atecc_init_done;
}

bool atecc_initialize(void)
{
    if (atecc_init_done) {
        // 初期化処理が実行済みの場合は終了
        return true;
    }
    fido_log_info("atecc_initialize start");

    // デバイスの初期化
    if (atecc_device_init() == false) {
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
        atecc_device_release();
        atecc_init_done = false;
        fido_log_info("atecc_finalize done");
    }
}

bool atecc_get_config_bytes(void)
{
    if (atecc_read_config_zone(work_buf_1) == false) {
        return false;
    }
#if LOG_HEXDUMP_DEBUG_CONFIG
    fido_log_debug("Config zone data (128 bytes):");
    fido_log_print_hexdump_debug(work_buf_1,      64);
    fido_log_print_hexdump_debug(work_buf_1 + 64, 64);
#endif
    return true;
}

//
// 外部秘密鍵の導入処理
//
bool atecc_install_privkey(uint8_t *privkey_raw_data)
{
    // 32バイトの一時キーを生成
    if (atecc_random(work_buf_2) == false) {
        fido_log_error("atecc_install_privkey failed: atcab_random returns false");
        return false;
    }

    // 一時キーを１５番スロットに書込み
    if (atecc_write_zone(ATECC_ZONE_DATA, KEY_ID_FOR_INSTALL_PRV_TMP_KEY, 0, 0, work_buf_2, ATECC_BLOCK_SIZE) == false) {
        fido_log_error("atecc_install_privkey failed: atcab_write_zone(%d) returns false", KEY_ID_FOR_INSTALL_PRV_TMP_KEY);
        return false;
    }

    // 秘密鍵を一時バッファにセット（先頭の４バイトは 0 埋めとする）
    memset(work_buf_1, 0x00, sizeof(work_buf_1));
    memcpy(work_buf_1 + 4, privkey_raw_data, ATECC_PRIV_KEY_SIZE);

    // 秘密鍵を１４番スロットに書込み
    //   １５番スロットには、１４番スロットに書込まれた
    //   秘密鍵を暗号化するキーが上書き保存されます。
    //   １４・１５番スロットの内容は、
    //   いかなる手段によっても参照することができません。
    if (atecc_priv_write(KEY_ID_FOR_INSTALL_PRIVATE_KEY, work_buf_1, KEY_ID_FOR_INSTALL_PRV_TMP_KEY, work_buf_2) == false) {
        fido_log_error("atecc_install_privkey failed: atecc_priv_write(%d) returns false", KEY_ID_FOR_INSTALL_PRIVATE_KEY);
        return false;
    }

    fido_log_debug("atecc_install_privkey success");
    return true;
}

bool atecc_generate_pubkey_from_privkey(uint8_t *public_key_buff)
{
    uint16_t key_id = KEY_ID_FOR_INSTALL_PRIVATE_KEY;
    memset(public_key_buff, 0x00, ATECC_PUB_KEY_SIZE);

    if (atecc_gen_key(GENKEY_MODE_PUBLIC, key_id, NULL, public_key_buff) == false) {
        fido_log_error("atecc_generate_pubkey_from_privkey failed: atecc_gen_key(%d) returns false", key_id);
        return false;
    }

    return true;
}
