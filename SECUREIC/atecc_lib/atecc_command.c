/* 
 * File:   atecc_command.c
 * Author: makmorit
 *
 * Created on 2020/08/11, 12:02
 */
#include <string.h>

#include "atecc_command.h"
#include "atecc_device.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#define ATECC_UNSUPPORTED_CMD ((uint16_t)0xFFFF)

static void atecc_command_crc(size_t length, const uint8_t *data, uint8_t *crc_le)
{
    size_t   counter;
    uint16_t crc_register = 0;
    uint16_t polynom = 0x8005;
    uint8_t  shift_register;
    uint8_t  data_bit;
    uint8_t  crc_bit;

    for (counter = 0; counter < length; counter++) {
        for (shift_register = 0x01; shift_register > 0x00; shift_register <<= 1) {
            data_bit = (data[counter] & shift_register) ? 1 : 0;
            crc_bit = crc_register >> 15;
            crc_register <<= 1;
            if (data_bit != crc_bit) {
                crc_register ^= polynom;
            }
        }
    }
    crc_le[0] = (uint8_t)(crc_register & 0x00FF);
    crc_le[1] = (uint8_t)(crc_register >> 8);
}

static void atecc_command_calc_crc(ATECC_PACKET *packet)
{
    // packet->param2 should be Little Endian
    uint8_t length = packet->txsize - ATECC_CRC_SIZE;
    // computer pointer to CRC in the packet
    uint8_t *crc = &(packet->txsize) + length;

    // stuff CRC into packet
    atecc_command_crc(length, &(packet->txsize), crc);
}

static bool atecc_command_check_crc(const uint8_t *response)
{
    uint8_t crc[ATECC_CRC_SIZE];
    uint8_t count = response[ATECC_IDX_COUNT];

    count -= ATECC_CRC_SIZE;
    atecc_command_crc(count, response, crc);

    return (crc[0] == response[count] && crc[1] == response[count + 1]);
}

bool atecc_command_lock(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_LOCK;
    packet->txsize = LOCK_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_read(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_READ;
    packet->txsize = READ_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_write(ATECC_COMMAND command, ATECC_PACKET *packet, bool has_mac)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_WRITE;

    packet->txsize = 7;
    if (packet->param1 & ATECC_ZONE_READWRITE_32) {
        packet->txsize += ATECC_BLOCK_SIZE;
    } else {
        packet->txsize += ATECC_WORD_SIZE;
    }
    if (has_mac) {
        packet->txsize += WRITE_MAC_SIZE;
    }
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_update_extra(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_UPDATE_EXTRA;
    packet->txsize = UPDATE_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_priv_write(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_PRIVWRITE;
    packet->txsize = PRIVWRITE_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_random(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_RANDOM;
    packet->txsize = RANDOM_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_nonce(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    // variable packet size
    uint8_t calc_mode = packet->param1 & NONCE_MODE_MASK;
    packet->opcode = ATECC_OP_NONCE;
    if ((calc_mode == NONCE_MODE_SEED_UPDATE || calc_mode == NONCE_MODE_NO_SEED_UPDATE)) {
        // Calculated nonce mode, 20 byte NumInm
        packet->txsize = NONCE_COUNT_SHORT;
    } else if (calc_mode == NONCE_MODE_PASSTHROUGH) {
        // PAss-through nonce mode
        if ((packet->param1 & NONCE_MODE_INPUT_LEN_MASK) == NONCE_MODE_INPUT_LEN_64) {
            // 64 byte NumIn
            packet->txsize = NONCE_COUNT_LONG_64;
        } else {
            // 32 byte NumIn
            packet->txsize = NONCE_COUNT_LONG;
        }
    } else {
        fido_log_error("atecc_command_nonce: BAD_PARAM");
        return false;
    }
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_gen_dig(ATECC_COMMAND command, ATECC_PACKET *packet, bool is_no_mac_key)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_GENDIG;
    if (packet->param1 == GENDIG_ZONE_SHARED_NONCE) {
        // shared nonce mode
        packet->txsize = GENDIG_COUNT + 32;
    } else if (is_no_mac_key) {
        // noMac keys use 4 bytes of OtherData in calculation
        packet->txsize = GENDIG_COUNT + 4;
    } else {
        packet->txsize = GENDIG_COUNT;
    }
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_gen_key(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_GENKEY;
    if (packet->param1 & GENKEY_MODE_PUBKEY_DIGEST) {
        packet->txsize = GENKEY_COUNT_DATA;
    } else {
        packet->txsize = GENKEY_COUNT;
    }
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_sign(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_SIGN;
    packet->txsize = SIGN_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_verify(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_VERIFY;

    // variable packet size based on mode
    switch (packet->param1 & VERIFY_MODE_MASK) {
        case VERIFY_MODE_STORED:
            packet->txsize = VERIFY_256_STORED_COUNT;
            break;
        case VERIFY_MODE_VALIDATE_EXTERNAL:
            packet->txsize = VERIFY_256_EXTERNAL_COUNT;
            break;
        case VERIFY_MODE_EXTERNAL:
            packet->txsize = VERIFY_256_EXTERNAL_COUNT;
            break;
        case VERIFY_MODE_VALIDATE:
        case VERIFY_MODE_INVALIDATE:
            packet->txsize = VERIFY_256_VALIDATE_COUNT;
            break;
        default:
            fido_log_error("atecc_command_verify: BAD_PARAM");
            return false;
    }

    atecc_command_calc_crc(packet);
    return true;
}

bool atecc_command_info(ATECC_COMMAND command, ATECC_PACKET *packet)
{
    // Set the opcode & parameters
    packet->opcode = ATECC_OP_INFO;
    packet->txsize = INFO_COUNT;
    atecc_command_calc_crc(packet);
    return true;
}

static bool atecc_command_is_error(uint8_t *data)
{
    // error packets are always 4 bytes long
    if (data[0] == 0x04) {
        switch (data[1]) {
        case 0x00:
            // No Error
            return true;
            break;
        case 0x01:
            fido_log_error("atecc_command_is_error: checkmac or verify failed");
            break;
        case 0x03:
            fido_log_error("atecc_command_is_error: command received byte length, opcode or parameter was illegal");
            break;
        case 0x05:
            fido_log_error("atecc_command_is_error: computation error during ECC processing causing invalid results");
            break;
        case 0x07:
            fido_log_error("atecc_command_is_error: chip is in self test failure mode");
            break;
        case 0x08:
            fido_log_error("atecc_command_is_error: random number generator health test error");
            break;
        case 0x0f: 
            fido_log_error("atecc_command_is_error: chip can't execute the command");
            break;
        case 0x11: 
            fido_log_error("atecc_command_is_error: chip was successfully woken up");
            break;
        case 0xff: 
            fido_log_error("atecc_command_is_error: bad crc found (command not properly received by device) or other comm error");
            break;
        default:
            fido_log_error("atecc_command_is_error: unknown error");
            break;
        }
        return false;

    } else {
        return true;
    }
}

// Execution times for ATECC508A supported commands
static const device_execution_time_t device_execution_time_508[] = {
    {ATECC_OP_CHECKMAC,     13},
    {ATECC_OP_COUNTER,      20},
    {ATECC_OP_DERIVE_KEY,   50},
    {ATECC_OP_ECDH,         58},
    {ATECC_OP_GENDIG,       11},
    {ATECC_OP_GENKEY,       115},
    {ATECC_OP_HMAC,         23},
    {ATECC_OP_INFO,         2},
    {ATECC_OP_LOCK,         32},
    {ATECC_OP_MAC,          14},
    {ATECC_OP_NONCE,        29},
    {ATECC_OP_PAUSE,        3},
    {ATECC_OP_PRIVWRITE,    48},
    {ATECC_OP_RANDOM,       23},
    {ATECC_OP_READ,         5},
    {ATECC_OP_SHA,          9},
    {ATECC_OP_SIGN,         60},
    {ATECC_OP_UPDATE_EXTRA, 10},
    {ATECC_OP_VERIFY,       72},
    {ATECC_OP_WRITE,        26}
};

// Execution times for ATECC608A-M0 supported commands
static const device_execution_time_t device_execution_time_608_m0[] = {
    {ATECC_OP_AES,          27},
    {ATECC_OP_CHECKMAC,     40},
    {ATECC_OP_COUNTER,      25},
    {ATECC_OP_DERIVE_KEY,   50},
    {ATECC_OP_ECDH,         75},
    {ATECC_OP_GENDIG,       25},
    {ATECC_OP_GENKEY,       115},
    {ATECC_OP_INFO,         5},
    {ATECC_OP_KDF,          165},
    {ATECC_OP_LOCK,         35},
    {ATECC_OP_MAC,          55},
    {ATECC_OP_NONCE,        20},
    {ATECC_OP_PRIVWRITE,    50},
    {ATECC_OP_RANDOM,       23},
    {ATECC_OP_READ,         5},
    {ATECC_OP_SECUREBOOT,   80},
    {ATECC_OP_SELFTEST,     250},
    {ATECC_OP_SHA,          36},
    {ATECC_OP_SIGN,         115},
    {ATECC_OP_UPDATE_EXTRA, 10},
    {ATECC_OP_VERIFY,       105},
    {ATECC_OP_WRITE,        45}
};

static bool get_exec_time(uint8_t opcode, ATECC_COMMAND command)
{
    bool status = true;
    const device_execution_time_t *execution_times;
    uint8_t no_of_commands;

    if (command->dt == ATECC608A && command->clock_divider == ATECC_CHIPMODE_CLOCK_DIV_M0) {
        // Assume default M0 clock divider
        execution_times = device_execution_time_608_m0;
        no_of_commands = sizeof(device_execution_time_608_m0) / sizeof(device_execution_time_t);
    } else {
        execution_times = device_execution_time_508;
        no_of_commands = sizeof(device_execution_time_508) / sizeof(device_execution_time_t);
    }

    command->execution_time_msec = ATECC_UNSUPPORTED_CMD;
    for (uint8_t i = 0; i < no_of_commands; i++) {
        if (execution_times[i].opcode == opcode) {
            command->execution_time_msec = execution_times[i].execution_time_msec;
            break;
        }
    }

    if (command->execution_time_msec == ATECC_UNSUPPORTED_CMD) {
        fido_log_error("get_exec_time failed: BAD_OPCODE");
        status = false;
    }

    return status;
}

bool atecc_command_execute(ATECC_PACKET* packet, ATECC_DEVICE device)
{
    bool status;
    uint32_t execution_or_wait_time;
    uint32_t max_delay_count;
    uint16_t rxsize;

    do {
        status = get_exec_time(packet->opcode, device->mCommands);
        if (status == false) {
            return status;
        }
        execution_or_wait_time = device->mCommands->execution_time_msec;
        max_delay_count = 0;

        bool wake_failed;
        status = atecc_iface_wake_func(device->mIface, &wake_failed);
        if (status == false) {
            return status;
        }

        // send the command
        status = atecc_iface_send_func(device->mIface, (uint8_t*)packet, packet->txsize);
        if (status == false) {
            break;
        }

        // Delay for execution time or initial wait before polling
        atecc_delay_ms(execution_or_wait_time);

        do {
            memset(packet->data, 0, sizeof(packet->data));
            // receive the response
            rxsize = sizeof(packet->data);
            status = atecc_iface_receive_func(device->mIface, packet->data, &rxsize);
            if (status == true) {
                break;
            }
        } while (max_delay_count-- > 0);
        if (status == false) {
            break;
        }

        // Check response size
        if (rxsize < 4) {
            if (rxsize > 0) {
                fido_log_error("atecc_command_execute failed: RX_FAIL");
                status = false;
            } else {
                fido_log_error("atecc_command_execute failed: RX_NO_RESPONSE");
                status = false;
            }
            break;
        }
        status = atecc_command_check_crc(packet->data);
        if (status == false) {
            break;
        }
        status = atecc_command_is_error(packet->data);
        if (status == false) {
            break;
        }

    } while (0);

    atecc_iface_idle_func(device->mIface);
    return status;
}
