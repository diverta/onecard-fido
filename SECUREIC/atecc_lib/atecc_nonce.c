/* 
 * File:   atecc_nonce.c
 * Author: makmorit
 *
 * Created on 2020/08/20, 12:52
 */
#include "atecc_command.h"
#include "atecc_nonce.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

bool atecc_calculate_nonce(atecc_nonce_in_out_t *param)
{
    uint8_t temporary[ATECC_MSG_SIZE_NONCE];
    uint8_t *p_temp;
    uint8_t calc_mode = param->mode & NONCE_MODE_MASK;

    // Check parameters
    if (param->temp_key == NULL || param->num_in == NULL) {
        fido_log_error("atecc_calculate_nonce failed: BAD_PARAM");
        return false;
    }

    // Calculate or pass-through the nonce to TempKey->Value
    if ((calc_mode == NONCE_MODE_SEED_UPDATE) || (calc_mode == NONCE_MODE_NO_SEED_UPDATE)) {
        // RandOut is only required for these modes
        if (param->rand_out == NULL) {
            fido_log_error("atecc_calculate_nonce failed: BAD_PARAM");
            return false;
        }

        if ((param->zero & NONCE_ZERO_CALC_MASK) == NONCE_ZERO_CALC_TEMPKEY) {
            // Nonce calculation mode. Actual value of TempKey has been returned in RandOut
            memcpy(param->temp_key->value, param->rand_out, 32);
            // TempKey flags aren't changed

        } else {
            // Calculate nonce using SHA-256 (refer to data sheet)
            p_temp = temporary;

            memcpy(p_temp, param->rand_out, RANDOM_NUM_SIZE);
            p_temp += RANDOM_NUM_SIZE;

            memcpy(p_temp, param->num_in, NONCE_NUMIN_SIZE);
            p_temp += NONCE_NUMIN_SIZE;

            *p_temp++ = ATECC_OP_NONCE;
            *p_temp++ = param->mode;
            *p_temp++ = 0x00;

            // Calculate SHA256 to get the nonce
            // atcac_sw_sha2_256(temporary, ATECC_MSG_SIZE_NONCE, param->temp_key->value);
            size_t hash_digest_size = 32;
            fido_crypto_generate_sha256_hash(temporary, ATECC_MSG_SIZE_NONCE, param->temp_key->value, &hash_digest_size);

            // Update TempKey flags
            param->temp_key->source_flag = 0; // Random
            param->temp_key->key_id = 0;
            param->temp_key->gen_dig_data = 0;
            param->temp_key->no_mac_flag = 0;
            param->temp_key->valid = 1;
        }

        // Update TempKey to only 32 bytes
        param->temp_key->is_64 = 0;

    } else if ((param->mode & NONCE_MODE_MASK) == NONCE_MODE_PASSTHROUGH) {
        if ((param->mode & NONCE_MODE_TARGET_MASK) == NONCE_MODE_TARGET_TEMPKEY) {
            // Pass-through mode for TempKey (other targets have no effect on TempKey)
            if ((param->mode & NONCE_MODE_INPUT_LEN_MASK) == NONCE_MODE_INPUT_LEN_64) {
                memcpy(param->temp_key->value, param->num_in, 64);
                param->temp_key->is_64 = 1;
            } else {
                memcpy(param->temp_key->value, param->num_in, 32);
                param->temp_key->is_64 = 0;
            }

            // Update TempKey flags
            param->temp_key->source_flag = 1; // Not Random
            param->temp_key->key_id = 0;
            param->temp_key->gen_dig_data = 0;
            param->temp_key->no_mac_flag = 0;
            param->temp_key->valid = 1;

        } else {
            // In the case of ECC608A, 
            // passthrough message may be stored in message digest buffer/ Alternate Key buffer
            // Update TempKey flags
            param->temp_key->source_flag = 1; //Not Random
            param->temp_key->key_id = 0;
            param->temp_key->gen_dig_data = 0;
            param->temp_key->no_mac_flag = 0;
            param->temp_key->valid = 0;

        }

    } else {
        fido_log_error("atecc_calculate_nonce failed: BAD_PARAM");
        return false;
    }

    return true;
}

static bool atecc_nonce_base(uint8_t mode, uint16_t zero, const uint8_t *num_in, uint8_t *rand_out)
{
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    uint8_t nonce_mode = mode & NONCE_MODE_MASK;

    // build a nonce command
    ATECC_PACKET packet;
    packet.param1 = mode;
    packet.param2 = zero;

    // Copy the right amount of NumIn data
    if ((nonce_mode == NONCE_MODE_SEED_UPDATE || nonce_mode == NONCE_MODE_NO_SEED_UPDATE)) {
        memcpy(packet.data, num_in, NONCE_NUMIN_SIZE);
    } else if (nonce_mode == NONCE_MODE_PASSTHROUGH) {
        if ((mode & NONCE_MODE_INPUT_LEN_MASK) == NONCE_MODE_INPUT_LEN_64) {
            memcpy(packet.data, num_in, 64);
        } else {
            memcpy(packet.data, num_in, 32);
        }
    } else {
        fido_log_error("atecc_nonce_base failed: BAD_PARAM");
        return false;
    }

    if (atecc_command_nonce(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    if ((rand_out != NULL) && (packet.data[ATECC_IDX_COUNT] >= 35)) {
        memcpy(&rand_out[0], &packet.data[ATECC_IDX_RSP_DATA], 32);
    }

    return true;
}

bool atecc_nonce_rand(const uint8_t *num_in, uint8_t *rand_out) 
{
    return atecc_nonce_base(NONCE_MODE_SEED_UPDATE, 0, num_in, rand_out);
}

bool atecc_nonce_load(uint8_t target, const uint8_t *num_in, uint16_t num_in_size)
{
    uint8_t mode = NONCE_MODE_PASSTHROUGH | (NONCE_MODE_TARGET_MASK & target);
    if (num_in_size == 32) {
        mode |= NONCE_MODE_INPUT_LEN_32;
    } else if (num_in_size == 64) {
        mode |= NONCE_MODE_INPUT_LEN_64;
    } else {
        fido_log_error("atecc_nonce_load failed: BAD_PARAM");
        return false;
    }
    return atecc_nonce_base(mode, 0, num_in, NULL);
}
