/* 
 * File:   fido_maintenance_attestation.c
 * Author: makmorit
 *
 * Created on 2023/01/13, 9:24
 */
//
// プラットフォーム非依存コード
//
#include "fido_command_common.h"
#include "fido_define.h"
#include "fido_maintenance.h"
#include "fido_maintenance_define.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(fido_maintenance_attestation);
#endif

// データ編集用エリア
static uint8_t work_buf[64];

//
// Install FIDO attestation
//
void fido_maintenance_attestation_install(void)
{
    uint8_t *data   = fido_maintenance_data_buffer();
    uint16_t length = fido_maintenance_data_buffer_size();

    // 元データチェック
    if (data == NULL || length == 0) {
        fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
        return;
    }
    fido_log_info("Install FIDO attestation start");

    // Flash ROMに登録する鍵・証明書データを準備
    if (fido_flash_skey_cert_data_prepare(data, length) == false) {
        fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
        return;
    }

    // 鍵・証明書データをFlash ROMへ書込
    if (fido_flash_skey_cert_write() == false) {
        fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
    }
}

static bool generate_random_password(void)
{
    // 32バイトのランダムベクターを生成
    fido_command_generate_random_vector(work_buf, 32);

    // Flash ROMに書き出して保存
    if (fido_flash_password_set(work_buf) == false) {
        return false;
    }

    fido_log_debug("Generated random vector for AES password ");
    return true;
}

void fido_maintenance_attestation_record_updated(void)
{
    if (fido_maintenance_command_byte() == MNT_COMMAND_INSTALL_ATTESTATION) {
        // 証明書データ書込完了
        fido_log_debug("Update FIDO attestation record completed ");

        // 続いて、AESパスワード生成処理を行う
        if (generate_random_password() == false) {
            fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
        }
    }
}

void fido_maintenance_attestation_aes_password_record_updated(void)
{
    if (fido_maintenance_command_byte() == MNT_COMMAND_INSTALL_ATTESTATION ||
        fido_maintenance_command_byte() == MNT_COMMAND_RESET_ATTESTATION) {
        // AESパスワード生成完了
        fido_log_debug("Update AES password record completed ");

        // レスポンスを生成してU2Fクライアントに戻す
        fido_maintenance_send_command_status(CTAP1_ERR_SUCCESS);
    }
}

//
// Erase FIDO attestation
//
void fido_maintenance_attestation_reset(void)
{
    fido_log_info("Reset FIDO attestation start");

    // 秘密鍵／証明書をFlash ROM領域から削除
    if (fido_flash_skey_cert_delete() == false) {
        fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
    }
}

void fido_maintenance_attestation_file_deleted(void)
{
    if (fido_maintenance_command_byte() == MNT_COMMAND_RESET_ATTESTATION) {
        // 秘密鍵／証明書削除が完了
        fido_log_debug("Erase FIDO attestation file completed ");

        // 続いて、署名カウンター情報をFlash ROM領域から削除
        if (fido_command_sign_counter_delete() == false) {
            fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
        }
    }
}

void fido_maintenance_attestation_token_counter_file_deleted(void)
{
    if (fido_maintenance_command_byte() == MNT_COMMAND_RESET_ATTESTATION) {
        // 署名カウンター情報削除が完了
        fido_log_debug("Erase token counter file completed");

        // 続いて、AESパスワード生成処理を行う
        if (generate_random_password() == false) {
            fido_maintenance_send_command_status(CTAP2_ERR_VENDOR_FIRST);
        }
    }
}
