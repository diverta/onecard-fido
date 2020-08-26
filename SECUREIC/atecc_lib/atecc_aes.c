/* 
 * File:   atecc_aes.c
 * Author: makmorit
 *
 * Created on 2020/08/24, 14:58
 */
#include "atecc.h"
#include "atecc_aes.h"
#include "atecc_command.h"
#include "atecc_nonce.h"
#include "atecc_read.h"
#include "atecc_util.h"
#include "atecc_write.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// (Key or TempKey){32} || (Challenge or TempKey){32} || OpCode{1} || Mode{1} || Param2{2} || 
// (OTP0_7 or 0){8} || (OTP8_10 or 0){3} || SN8{1} || (SN4_7 or 0){4} || SN0_1{2} || (SN2_3 or 0){2}
#define ATECC_MSG_SIZE_MAC  (88)

// 作業用一時領域
static uint8_t mac_message_buf[ATECC_MSG_SIZE_MAC];
static uint8_t response[MAC_SIZE];
static uint8_t other_data[CHECKMAC_OTHER_DATA_SIZE];
static uint8_t num_in[NONCE_NUMIN_SIZE];
static uint8_t rand_out[RANDOM_NUM_SIZE];
static uint8_t serial_number[ATECC_SERIAL_NUM_SIZE];

static atecc_temp_key_t temp_key;
static atecc_nonce_in_out_t nonce_params;
static atecc_check_mac_in_out_t checkmac_params;

//
// Persistent Latch設定
//
static bool atecc_check_mac_calc_resp(atecc_check_mac_in_out_t *param)
{
    // Check parameters
    if (param == NULL || param->other_data == NULL || param->sn == NULL || param->client_resp == NULL) {
        fido_log_error("atecc_check_mac_calc_resp failed: BAD_PARAM");
        return false;
    }

    bool is_temp_key_req = false;
    if ((param->mode & CHECKMAC_MODE_BLOCK1_TEMPKEY) || (param->mode & CHECKMAC_MODE_BLOCK2_TEMPKEY)) {
        is_temp_key_req = true;  // Message uses TempKey
    } else if ((param->mode == 0x01 || param->mode == 0x05) && param->target_key != NULL) {
        is_temp_key_req = true;  // CheckMac copy will be performed

    }
    if (is_temp_key_req && param->temp_key == NULL) {
        fido_log_error("atecc_check_mac_calc_resp failed: BAD_PARAM");
        return false;
    }
    if (!(param->mode & CHECKMAC_MODE_BLOCK1_TEMPKEY) && param->slot_key == NULL) {
        fido_log_error("atecc_check_mac_calc_resp failed: BAD_PARAM");
        return false;
    }
    if (!(param->mode & CHECKMAC_MODE_BLOCK2_TEMPKEY) && param->client_chal == NULL) {
        fido_log_error("atecc_check_mac_calc_resp failed: BAD_PARAM");
        return false;
    }
    if ((param->mode & CHECKMAC_MODE_INCLUDE_OTP_64) && param->otp == NULL) {
        fido_log_error("atecc_check_mac_calc_resp failed: BAD_PARAM");
        return false;
    }

    if ((param->mode & CHECKMAC_MODE_BLOCK1_TEMPKEY) || (param->mode & CHECKMAC_MODE_BLOCK2_TEMPKEY)) {
        // This will use TempKey in message, check validity
        if (!param->temp_key->valid) {
            fido_log_error("atecc_check_mac_calc_resp failed: EXECUTION_ERROR (TempKey is not valid)");
            return false;
        }
        if (((param->mode >> 2) & 0x01) != param->temp_key->source_flag) {
            fido_log_error("atecc_check_mac_calc_resp failed: EXECUTION_ERROR (TempKey SourceFlag doesn't match bit 2 of the mode)");
            return false;
        }
    }

    // Build the message
    uint8_t *msg = mac_message_buf;
    memset(msg, 0, sizeof(msg));
    if (param->mode & CHECKMAC_MODE_BLOCK1_TEMPKEY) {
        memcpy(&msg[0], param->temp_key->value, 32);
    } else {
        memcpy(&msg[0], param->slot_key, 32);
    }
    if (param->mode & CHECKMAC_MODE_BLOCK2_TEMPKEY) {
        memcpy(&msg[32], param->temp_key->value, 32);
    } else {
        memcpy(&msg[32], param->client_chal, 32);
    }
    memcpy(&msg[64], &param->other_data[0], 4);
    if (param->mode & CHECKMAC_MODE_INCLUDE_OTP_64) {
        memcpy(&msg[68], param->otp, 8);
    }
    memcpy(&msg[76], &param->other_data[4], 3);
    msg[79] = param->sn[8];
    memcpy(&msg[80], &param->other_data[7], 4);
    memcpy(&msg[84], &param->sn[0], 2);
    memcpy(&msg[86], &param->other_data[11], 2);

    // Calculate the client response
    // atcac_sw_sha2_256(msg, sizeof(msg), param->client_resp);
    size_t hash_digest_size = 32;
    fido_crypto_generate_sha256_hash(msg, ATECC_MSG_SIZE_MAC, param->client_resp, &hash_digest_size);

    // Update TempKey fields
    if ((param->mode == 0x01 || param->mode == 0x05) && param->target_key != NULL) {
        // CheckMac Copy will be performed
        memcpy(param->temp_key->value, param->target_key, ATECC_KEY_SIZE);
        param->temp_key->gen_dig_data = 0;
        param->temp_key->source_flag = 1;
        param->temp_key->valid = 1;
    }

    return true;
}

static bool atecc_check_mac(uint8_t mode, uint16_t key_id, const uint8_t *challenge, const uint8_t *response, const uint8_t *other_data)
{
    // Verify the inputs
    if (response == NULL || other_data == NULL) {
        fido_log_error("atecc_check_mac failed: BAD_PARAM");
        return false;
    }
    if (!(mode & CHECKMAC_MODE_BLOCK2_TEMPKEY) && challenge == NULL) {
        fido_log_error("atecc_check_mac failed: BAD_PARAM");
        return false;
    }

    // build Check MAC command
    ATECC_PACKET packet;
    packet.param1 = mode;
    packet.param2 = key_id;
    if (challenge != NULL) {
        memcpy(&packet.data[0], challenge, CHECKMAC_CLIENT_CHALLENGE_SIZE);
    } else {
        memset(&packet.data[0], 0, CHECKMAC_CLIENT_CHALLENGE_SIZE);
    }
    memcpy(&packet.data[32], response, CHECKMAC_CLIENT_RESPONSE_SIZE);
    memcpy(&packet.data[64], other_data, CHECKMAC_OTHER_DATA_SIZE);

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_check_mac(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    return true;
}

bool atecc_aes_set_persistent_latch(uint16_t write_key_id, uint8_t *write_key)
{
    //
    // ATECC608Aの内部スロットに格納された鍵により、
    // AES関連処理を実行するため、
    // Persistent Latchに 1 を設定します。
    //
    // Perform random nonce
    memset(&temp_key, 0, sizeof(temp_key));
    memset(num_in, 0, sizeof(num_in));
    memset(&nonce_params, 0, sizeof(nonce_params));
    nonce_params.mode = NONCE_MODE_SEED_UPDATE;
    nonce_params.zero = 0;
    nonce_params.num_in = num_in;
    nonce_params.rand_out = rand_out;
    nonce_params.temp_key = &temp_key;
    if (atecc_nonce_rand(nonce_params.num_in, rand_out) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_nonce_rand returns false");
        return false;
    }

    // Calculate nonce value
    if (atecc_calculate_nonce(&nonce_params) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_calculate_nonce returns false");
        return false;
    }

    // Read SN
    if (atecc_read_serial_number(serial_number) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_read_serial_number returns false");
        return false;
    }

    // Calculate response
    for (int i = 0; i < sizeof(other_data); i++) {
        other_data[i] = (uint8_t)(i + 0xF0);
    }
    checkmac_params.mode = CHECKMAC_MODE_BLOCK2_TEMPKEY;
    checkmac_params.key_id = write_key_id;
    checkmac_params.client_chal = NULL;
    checkmac_params.client_resp = response;
    checkmac_params.other_data = other_data;
    checkmac_params.sn = serial_number;
    checkmac_params.otp = NULL;
    checkmac_params.slot_key = write_key;
    checkmac_params.target_key = NULL;
    checkmac_params.temp_key = &temp_key;
    if (atecc_check_mac_calc_resp(&checkmac_params) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_check_mac_calc_resp returns false");
        return false;
    }
    
    // Perform CheckMac
    if (atecc_check_mac(checkmac_params.mode, checkmac_params.key_id, checkmac_params.client_chal, checkmac_params.client_resp, checkmac_params.other_data) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_check_mac returns false");
        return false;
    }

    if (atecc_info_set_latch(true) == false) {
        fido_log_error("atecc_aes_set_persistent_latch failed: atecc_info_set_latch returns false");
        return false;
    }

    return true;
}

//
// 暗号化／復号化
//
static bool atecc_aes(uint8_t mode, uint16_t key_id, uint8_t *aes_in, uint8_t *aes_out)
{
    if (aes_in == NULL) {
        fido_log_error("atecc_aes failed: BAD_PARAM");
        return false;
    }

    // build a AES command
    ATECC_PACKET packet;
    packet.param1 = mode;
    packet.param2 = key_id;
    if (AES_MODE_GFM == (mode & AES_MODE_GFM)) {
        memcpy(packet.data, aes_in, ATECC_AES_GFM_SIZE);
    } else {
        memcpy(packet.data, aes_in, AES_DATA_SIZE);
    }

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_aes(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    if (aes_out && packet.data[ATECC_IDX_COUNT] >= (3 + AES_DATA_SIZE)) {
        // The AES command return a 16 byte data.
        memcpy(aes_out, &packet.data[ATECC_IDX_RSP_DATA], AES_DATA_SIZE);
    }

    return true;
}

bool atecc_aes_encrypt(uint8_t *plaintext, size_t plaintext_size, uint8_t *encrypted)
{
    // 変数の初期化
    uint16_t aes_key_id = KEY_ID_FOR_INSTALL_AES_PASSWORD;
    size_t   data_block_num = plaintext_size / AES_DATA_SIZE;

    // Encryption with the AES keys
    for (size_t data_block = 0; data_block < data_block_num; data_block++) {
        // 暗号ブロックを更新
        uint8_t aes_key_block = data_block % 2;

        // データブロックごとに暗号化
        uint8_t mode = AES_MODE_ENCRYPT | (AES_MODE_KEY_BLOCK_MASK & (aes_key_block << AES_MODE_KEY_BLOCK_POS));
        size_t offset = data_block * AES_DATA_SIZE;
        if (atecc_aes(mode, aes_key_id, plaintext + offset, encrypted + offset) == false) {
            fido_log_error("atecc_aes_encrypt failed: atecc_aes(%d) returns false", aes_key_id);
            return false;
        }
    }

    return true;
}

bool atecc_aes_decrypt(uint8_t *encrypted, size_t encrypted_size, uint8_t *decrypted)
{
    // 変数の初期化
    uint16_t aes_key_id = KEY_ID_FOR_INSTALL_AES_PASSWORD;
    size_t   data_block_num = encrypted_size / AES_DATA_SIZE;

    // Decryption with the AES keys
    for (size_t data_block = 0; data_block < data_block_num; data_block++) {
        // 暗号ブロックを更新
        uint8_t aes_key_block = data_block % 2;

        // データブロックごとに復号化
        uint8_t mode = AES_MODE_DECRYPT | (AES_MODE_KEY_BLOCK_MASK & (aes_key_block << AES_MODE_KEY_BLOCK_POS));
        size_t offset = data_block * AES_DATA_SIZE;
        if (atecc_aes(mode, aes_key_id, encrypted + offset, decrypted + offset) == false) {
            fido_log_error("atecc_aes_decrypt failed: atecc_aes(%d) returns false", aes_key_id);
            return false;
        }
    }

    return true;
}

void atecc_aes_test(void)
{
    fido_log_info("atecc_aes_test done");
}
