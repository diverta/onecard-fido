/* 
 * File:   atecc_priv.c
 * Author: makmorit
 *
 * Created on 2020/08/19, 14:32
 */
#include "atecc_command.h"
#include "atecc_device.h"
#include "atecc_read.h"
#include "atecc_util.h"
#include "atecc_write.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// RandOut{32} || NumIn{20} || OpCode{1} || Mode{1} || LSB of Param2{1}
#define ATECC_MSG_SIZE_NONCE            (55)
//! KeyId{32} || OpCode{1} || Param1{1} || Param2{2} || SN8{1} || SN0_1{2} || 0{25} || TempKey{32}
#define ATECC_MSG_SIZE_GEN_DIG          (96)
//! KeyId{32} || OpCode{1} || Param1{1} || Param2{2}|| SN8{1} || SN0_1{2} || 0{21} || PlainText{36}
#define ATCA_MSG_SIZE_PRIVWRITE_MAC     (96)
#define ATCA_PRIVWRITE_MAC_ZEROS_SIZE   (21)
#define ATCA_PRIVWRITE_PLAIN_TEXT_SIZE  (36)


typedef struct atca_nonce_in_out {
    uint8_t               mode;
    uint16_t              zero;
    const uint8_t        *num_in;
    const uint8_t        *rand_out;
    struct atca_temp_key *temp_key;
} atca_nonce_in_out_t;

typedef struct atca_gen_dig_in_out {
    uint8_t               zone;             // [in] Zone/Param1 for the GenDig command
    uint16_t              key_id;           // [in] KeyId/Param2 for the GenDig command
    uint16_t              slot_conf;        // [in] Slot config for the GenDig command
    uint16_t              key_conf;         // [in] Key config for the GenDig command
    uint8_t               slot_locked;      // [in] slot locked for the GenDig command
    uint32_t              counter;          // [in] counter for the GenDig command
    bool                  is_key_nomac;     // [in] Set to true if the slot pointed to be key_id has the SotConfig.NoMac bit set
    const uint8_t        *sn;               // [in] Device serial number SN[0:8]. Only SN[0:1] and SN[8] are required though.
    const uint8_t        *stored_value;     // [in] 32-byte slot value, config block, OTP block as specified by the Zone/KeyId parameters
    const uint8_t        *other_data;       // [in] 32-byte value for shared nonce zone, 4-byte value if is_key_nomac is true, ignored and/or NULL otherwise
    struct atca_temp_key *temp_key;         // [inout] Current state of TempKey
} atca_gen_dig_in_out_t;

typedef struct atca_write_mac_in_out {
    uint8_t               zone;             // Zone/Param1 for the Write or PrivWrite command
    uint16_t              key_id;           // KeyID/Param2 for the Write or PrivWrite command
    const uint8_t        *sn;               // Device serial number SN[0:8]. Only SN[0:1] and SN[8] are required though.
    const uint8_t        *input_data;       // Data to be encrypted. 32 bytes for Write command, 36 bytes for PrivWrite command.
    uint8_t              *encrypted_data;   // Encrypted version of input_data will be returned here. 32 bytes for Write command, 36 bytes for PrivWrite command.
    uint8_t              *auth_mac;         // Write MAC will be returned here. 32 bytes.
    struct atca_temp_key *temp_key;         // Current state of TempKey.
} atca_write_mac_in_out_t;

typedef struct atca_temp_key {
    uint8_t  value[ATECC_KEY_SIZE * 2];     // Value of TempKey (64 bytes for ATECC608A only)
    unsigned key_id       : 4;              // If TempKey was derived from a slot or transport key (GenDig or GenKey), that key ID is saved here.
    unsigned source_flag  : 1;              // Indicates id TempKey started from a random nonce (0) or not (1).
    unsigned gen_dig_data : 1;              // TempKey was derived from the GenDig command.
    unsigned gen_key_data : 1;              // TempKey was derived from the GenKey command (ATECC devices only).
    unsigned no_mac_flag  : 1;              // TempKey was derived from a key that has the NoMac bit set preventing the use of the MAC command. Known as CheckFlag in ATSHA devices).
    unsigned valid        : 1;              // TempKey is valid.
    uint8_t  is_64;                         // TempKey has 64 bytes of valid data
} atca_temp_key_t;


static bool atecc_do_nonce(struct atca_nonce_in_out *param)
{
    uint8_t temporary[ATECC_MSG_SIZE_NONCE];
    uint8_t *p_temp;
    uint8_t calc_mode = param->mode & NONCE_MODE_MASK;

    // Check parameters
    if (param->temp_key == NULL || param->num_in == NULL) {
        fido_log_error("atcah_nonce failed: BAD_PARAM");
        return false;
    }

    // Calculate or pass-through the nonce to TempKey->Value
    if ((calc_mode == NONCE_MODE_SEED_UPDATE) || (calc_mode == NONCE_MODE_NO_SEED_UPDATE)) {
        // RandOut is only required for these modes
        if (param->rand_out == NULL) {
            fido_log_error("atcah_nonce failed: BAD_PARAM");
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
        fido_log_error("atcah_nonce failed: BAD_PARAM");
        return false;
    }

    return true;
}

static bool atecc_nonce_base(uint8_t mode, uint16_t zero, const uint8_t *num_in, uint8_t* rand_out)
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

static bool atecc_nonce_rand(const uint8_t *num_in, uint8_t* rand_out) 
{
    return atecc_nonce_base(NONCE_MODE_SEED_UPDATE, 0, num_in, rand_out);
}

static bool atecc_gen_dig(uint8_t zone, uint16_t key_id, const uint8_t *other_data, uint8_t other_data_size)
{
    if (other_data_size > 0 && other_data == NULL) {
        fido_log_error("atecc_gendig failed: BAD_PARAM");
        return false;
    }

    // build gendig command
    ATECC_PACKET packet;
    packet.param1 = zone;
    packet.param2 = key_id;

    bool is_no_mac_key = false;
    if (packet.param1 == GENDIG_ZONE_SHARED_NONCE && other_data_size >= ATECC_BLOCK_SIZE) {
        memcpy(&packet.data[0], &other_data[0], ATECC_BLOCK_SIZE);
    } else if (packet.param1 == GENDIG_ZONE_DATA && other_data_size >= ATECC_WORD_SIZE) {
        memcpy(&packet.data[0], &other_data[0], ATECC_WORD_SIZE);
        is_no_mac_key = true;
    }

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_gen_dig(command, &packet, is_no_mac_key) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    return true;
}

static bool atecc_do_gen_dig(struct atca_gen_dig_in_out *param)
{
    uint8_t temporary[ATECC_MSG_SIZE_GEN_DIG];
    uint8_t *p_temp;

    // Check parameters
    if (param->sn == NULL || param->temp_key == NULL) {
        fido_log_error("atcah_gen_dig failed: BAD_PARAM");
        return false;
    }
    if ((param->zone <= GENDIG_ZONE_DATA) && (param->stored_value == NULL)) {
        fido_log_error("atcah_gen_dig failed: Stored value cannot be null for Config, OTP and Data");
        return false;
    }
    if ((param->zone == GENDIG_ZONE_SHARED_NONCE || (param->zone == GENDIG_ZONE_DATA && param->is_key_nomac)) && param->other_data == NULL) {
        fido_log_error("atcah_gen_dig failed: Other data is required in these cases");
        return false;
    }
    if (param->zone > 5) {
        fido_log_error("atcah_gen_dig failed: Unknown zone");
        return false;
    }
    // Start calculation
    p_temp = temporary;

    // (1) 32 bytes inputKey
    if (param->zone == GENDIG_ZONE_SHARED_NONCE) {
        if (param->key_id & 0x8000) {
            // 32 bytes TempKey
            memcpy(p_temp, param->temp_key->value, ATECC_KEY_SIZE);
        } else {
            // 32 bytes other data
            memcpy(p_temp, param->other_data, ATECC_KEY_SIZE);
        }
    } else if (param->zone == GENDIG_ZONE_COUNTER || param->zone == GENDIG_ZONE_KEY_CONFIG) {
        // 32 bytes of zero.
        memset(p_temp, 0x00, ATECC_KEY_SIZE);
    } else {
        // 32 bytes of stored data
        memcpy(p_temp, param->stored_value, ATECC_KEY_SIZE);
    }
    p_temp += ATECC_KEY_SIZE;

    if (param->zone == GENDIG_ZONE_DATA && param->is_key_nomac) {
        // If a key has the SlotConfig.NoMac bit set, then opcode and parameters come from OtherData
        memcpy(p_temp, param->other_data, 4);
        p_temp += 4;
    } else {
        // (2) 1 byte Opcode
        *p_temp++ = ATECC_OP_GENDIG;

        // (3) 1 byte Param1 (zone)
        *p_temp++ = param->zone;

        // (4) 1 byte LSB of Param2 (keyID)
        *p_temp++ = (uint8_t)(param->key_id & 0xFF);
        if (param->zone == GENDIG_ZONE_SHARED_NONCE) {
            //(4) 1 byte zero for shared nonce mode
            *p_temp++ = 0;
        } else {
            //(4)  1 byte MSB of Param2 (keyID) for other modes
            *p_temp++ = (uint8_t)(param->key_id >> 8);
        }
    }

    // (5) 1 byte SN[8]
    *p_temp++ = param->sn[8];

    // (6) 2 bytes SN[0:1]
    *p_temp++ = param->sn[0];
    *p_temp++ = param->sn[1];

    // (7)
    if (param->zone == GENDIG_ZONE_COUNTER) {
        *p_temp++ = 0;
        // (7) 4 bytes of counter
        *p_temp++ = (uint8_t)(param->counter & 0xFF);
        *p_temp++ = (uint8_t)(param->counter >> 8);
        *p_temp++ = (uint8_t)(param->counter >> 16);
        *p_temp++ = (uint8_t)(param->counter >> 24);

        // (7) 20 bytes of zero
        memset(p_temp, 0x00, 20);
        p_temp += 20;

    } else if (param->zone == GENDIG_ZONE_KEY_CONFIG) {
        *p_temp++ = 0;
        // (7) 2 bytes of Slot config
        *p_temp++ = param->slot_conf & 0xFF;
        *p_temp++ = (uint8_t)(param->slot_conf >> 8);

        // (7) 2 bytes of key config
        *p_temp++ = param->key_conf & 0xFF;
        *p_temp++ = (uint8_t)(param->key_conf >> 8);

        // (7) 1 byte of slot locked
        *p_temp++ = param->slot_locked;

        // (7) 19 bytes of zero
        memset(p_temp, 0x00, 19);
        p_temp += 19;

    } else {
        // (7) 25 zeros
        memset(p_temp, 0, 25);
        p_temp += 25;
    }

    if (param->zone == GENDIG_ZONE_SHARED_NONCE && (param->key_id & 0x8000)) {
        // (8) 32 bytes OtherData
        memcpy(p_temp, param->other_data, ATECC_KEY_SIZE);
        p_temp += ATECC_KEY_SIZE;

    } else {
        // (8) 32 bytes TempKey
        memcpy(p_temp, param->temp_key->value, ATECC_KEY_SIZE);
        p_temp += ATECC_KEY_SIZE;
    }

    // Calculate SHA256 to get the new TempKey
    // atcac_sw_sha2_256(temporary, (p_temp - temporary), param->temp_key->value);
    size_t hash_digest_size = 32;
    fido_crypto_generate_sha256_hash(temporary, (p_temp - temporary), param->temp_key->value, &hash_digest_size);

    // Update TempKey fields
    param->temp_key->valid = 1;

    if ((param->zone == GENDIG_ZONE_DATA) && (param->key_id <= 15)) {
        param->temp_key->gen_dig_data = 1;
        param->temp_key->key_id = (param->key_id & 0xF);    // mask lower 4-bit only
        if (param->is_key_nomac == 1) {
            param->temp_key->no_mac_flag = 1;
        }
    } else {
        param->temp_key->gen_dig_data = 0;
        param->temp_key->key_id = 0;
    }

    return true;
}

static bool atecc_privwrite_auth_mac(struct atca_write_mac_in_out *param)
{
    uint8_t mac_input[ATCA_MSG_SIZE_PRIVWRITE_MAC];
    uint8_t i = 0;
    uint8_t *p_temp = NULL;
    uint8_t session_key2[32];

    // Check parameters
    if (!param->input_data || !param->temp_key) {
        fido_log_error("atecc_privwrite_auth_mac failed: BAD_PARAM");
        return false;
    }

    // Check TempKey fields validity (TempKey is always used)
    // TempKey.CheckFlag must be 0 and TempKey.Valid must be 1
    if (param->temp_key->no_mac_flag || (param->temp_key->valid != 1)) {
        // Invalidate TempKey, then return
        param->temp_key->valid = 0;
        fido_log_error("atecc_privwrite_auth_mac failed: EXECUTION_ERROR");
        return false;
    }

    //
    // Encrypt by XOR-ing Data with the TempKey
    //
    // Encrypt the next 28 bytes of the cipher text, 
    // which is the first part of the private key.
    for (i = 0; i < 32; i++) {
        param->encrypted_data[i] = param->input_data[i] ^ param->temp_key->value[i];
    }

    // Calculate the new key for the last 4 bytes of the cipher text
    // atcac_sw_sha2_256(param->temp_key->value, 32, session_key2);
    size_t hash_digest_size = 32;
    fido_crypto_generate_sha256_hash(param->temp_key->value, 32, session_key2, &hash_digest_size);

    // Encrypt the last 4 bytes of the cipher text, 
    // which is the remaining part of the private key
    for (i = 32; i < 36; i++) {
        param->encrypted_data[i] = param->input_data[i] ^ session_key2[i - 32];
    }

    // If the pointer *mac is provided by the caller then calculate input MAC
    if (param->auth_mac) {
        // Start calculation
        p_temp = mac_input;

        // (1) 32 bytes TempKey
        memcpy(p_temp, param->temp_key->value, ATECC_KEY_SIZE);
        p_temp += ATECC_KEY_SIZE;

        // (2) 1 byte Opcode
        *p_temp++ = ATECC_OP_PRIVWRITE;

        // (3) 1 byte Param1 (zone)
        *p_temp++ = param->zone;

        // (4) 2 bytes Param2 (keyID)
        *p_temp++ = param->key_id & 0xFF;
        *p_temp++ = (param->key_id >> 8) & 0xFF;

        // (5) 1 byte SN[8]
        *p_temp++ = param->sn[8];

        // (6) 2 bytes SN[0:1]
        *p_temp++ = param->sn[0];
        *p_temp++ = param->sn[1];

        // (7) 21 zeros
        memset(p_temp, 0, ATCA_PRIVWRITE_MAC_ZEROS_SIZE);
        p_temp += ATCA_PRIVWRITE_MAC_ZEROS_SIZE;

        // (8) 36 bytes PlainText (Private Key)
        memcpy(p_temp, param->input_data, ATCA_PRIVWRITE_PLAIN_TEXT_SIZE);

        // Calculate SHA256 to get the new TempKey
        // atcac_sw_sha2_256(mac_input, sizeof(mac_input), param->auth_mac);
        fido_crypto_generate_sha256_hash(mac_input, sizeof(mac_input), param->auth_mac, &hash_digest_size);
    }

    return true;
}

bool atecc_priv_write(uint16_t key_id, const uint8_t priv_key[36], uint16_t write_key_id, const uint8_t write_key[32])
{
    ATECC_PACKET packet;
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = false;
    atca_nonce_in_out_t nonce_params;
    atca_gen_dig_in_out_t gen_dig_param;
    atca_write_mac_in_out_t host_mac_param;
    atca_temp_key_t temp_key;
    uint8_t serial_num[32]; // Buffer is larger than the 9 bytes required to make reads easier
    uint8_t num_in[NONCE_NUMIN_SIZE] = { 0 };
    uint8_t rand_out[RANDOM_NUM_SIZE] = { 0 };
    uint8_t cipher_text[36] = { 0 };
    uint8_t host_mac[MAC_SIZE] = { 0 };
    uint8_t other_data[4] = { 0 };

    if (key_id > 15 || priv_key == NULL) {
        fido_log_error("atecc_write failed: BAD_PARAM");
        return false;
    }

    if (write_key == NULL) {
        // Caller requested an unencrypted PrivWrite, 
        // which is only allowed when the data zone is unlocked
        // build an PrivWrite command
        packet.param1 = 0x00;                           // Mode is unencrypted write
        packet.param2 = key_id;                         // Key ID
        memcpy(&packet.data[0], priv_key, 36);          // Private key
        memset(&packet.data[36], 0, 32);                // MAC (ignored for unencrypted write)

    } else {
        // Read the device SN
        if ((status = atecc_read_zone(ATECC_ZONE_CONFIG, 0, 0, 0, serial_num, 32)) == false) {
            return false;
        }
        // Make the SN continuous by moving SN[4:8] right after SN[0:3]
        memmove(&serial_num[4], &serial_num[8], 5);

        // Send the random Nonce command
        if ((status = atecc_nonce_rand(num_in, rand_out)) == false) {
            return false;
        }

        // Calculate Tempkey
        memset(&temp_key, 0, sizeof(temp_key));
        memset(&nonce_params, 0, sizeof(nonce_params));
        nonce_params.mode = NONCE_MODE_SEED_UPDATE;
        nonce_params.zero = 0;
        nonce_params.num_in = num_in;
        nonce_params.rand_out = rand_out;
        nonce_params.temp_key = &temp_key;
        if (atecc_do_nonce(&nonce_params) == false) {
            return false;
        }

        // Supply OtherData so GenDig behavior is the same for keys with SlotConfig.NoMac set
        other_data[0] = ATECC_OP_GENDIG;
        other_data[1] = GENDIG_ZONE_DATA;
        other_data[2] = (uint8_t)(write_key_id);
        other_data[3] = (uint8_t)(write_key_id >> 8);

        // Send the GenDig command
        if (atecc_gen_dig(GENDIG_ZONE_DATA, write_key_id, other_data, sizeof(other_data)) == false) {
            return false;
        }

        // Calculate Tempkey
        // NoMac bit isn't being considered here on purpose to remove having to read SlotConfig.
        // OtherData is built to get the same result regardless of the NoMac bit.
        memset(&gen_dig_param, 0, sizeof(gen_dig_param));
        gen_dig_param.zone = GENDIG_ZONE_DATA;
        gen_dig_param.sn = serial_num;
        gen_dig_param.key_id = write_key_id;
        gen_dig_param.is_key_nomac = false;
        gen_dig_param.stored_value = write_key;
        gen_dig_param.other_data = other_data;
        gen_dig_param.temp_key = &temp_key;
        if (atecc_do_gen_dig(&gen_dig_param) == false) {
            return false;
        }

        // Calculate Auth MAC and cipher text
        memset(&host_mac_param, 0, sizeof(host_mac_param));
        host_mac_param.zone = PRIVWRITE_MODE_ENCRYPT;
        host_mac_param.key_id = key_id;
        host_mac_param.sn = serial_num;
        host_mac_param.input_data = &priv_key[0];
        host_mac_param.encrypted_data = cipher_text;
        host_mac_param.auth_mac = host_mac;
        host_mac_param.temp_key = &temp_key;
        if (atecc_privwrite_auth_mac(&host_mac_param) == false) {
            return false;
        }

        // build a write command for encrypted writes
        packet.param1 = PRIVWRITE_MODE_ENCRYPT;            // Mode is encrypted write
        packet.param2 = key_id;                            // Key ID
        memcpy(&packet.data[0], cipher_text, sizeof(cipher_text));
        memcpy(&packet.data[sizeof(cipher_text)], host_mac, sizeof(host_mac));
    }

    if ((status = atecc_command_priv_write(command, &packet)) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    return true;
}
