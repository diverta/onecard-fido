/* 
 * File:   atecc_util.c
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:36
 */
#include <stdlib.h>
#include <string.h>

#include "atecc_command.h"
#include "atecc_util.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

bool atecc_get_address(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t *addr)
{
    uint8_t mem_zone = zone & 0x03;

    if (addr == NULL) {
        fido_log_error("atecc_get_address failed: BAD_PARAM");
        return false;
    }
    if ((mem_zone != ATECC_ZONE_CONFIG) && (mem_zone != ATECC_ZONE_DATA) && (mem_zone != ATECC_ZONE_OTP)) {
        fido_log_error("atecc_get_address failed: BAD_PARAM");
        return false;
    }

    // Initialize the addr to 00
    *addr = 0;
    // Mask the offset
    offset = offset & (uint8_t)0x07;
    if ((mem_zone == ATECC_ZONE_CONFIG) || (mem_zone == ATECC_ZONE_OTP)) {
        *addr = block << 3;
        *addr |= offset;
    } else {
        // ATECC_ZONE_DATA
        *addr = slot << 3;
        *addr  |= offset;
        *addr |= block << 8;
    }

    return true;
}

bool atecc_get_zone_size(uint8_t zone, uint16_t slot, size_t *size)
{
    bool status = true;

    if (size == NULL) {
        fido_log_error("atecc_get_zone_size failed: BAD_PARAM (size == NULL)");
        return false;
    }

    switch (zone) {
        case ATECC_ZONE_CONFIG:
            *size = 128;
            break;
        case ATECC_ZONE_OTP:
            *size = 64;
            break;
        case ATECC_ZONE_DATA:
            if (slot < 8) {
                *size = 36;
            } else if (slot == 8) {
                *size = 416;
            } else if (slot < 16) {
                *size = 72;
            } else {
                fido_log_error("atecc_get_zone_size failed: BAD_PARAM (invalid ATECC_ZONE_DATA)");
                status = false;
            }
            break;
        default:
            fido_log_error("atecc_get_zone_size failed: BAD_PARAM");
            status = false;
            break;
    }

    return status;
}

//
// ロック関連
//
static bool atecc_lock(uint8_t mode, uint16_t summary_crc)
{
    // build command for lock zone and send
    ATECC_PACKET packet;
    memset(&packet, 0, sizeof(packet));
    packet.param1 = mode;
    packet.param2 = summary_crc;

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = atecc_command_lock(command, &packet);
    if (status == false) {
        return status;
    }

    status = atecc_command_execute(&packet, atecc_device_ref());
    if (status == false) {
        return status;
    }

    return true;
}

bool atecc_lock_config_zone(void)
{
    return atecc_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_CONFIG, 0);
}

bool atecc_lock_data_zone(void)
{
    return atecc_lock(LOCK_ZONE_NO_CRC | LOCK_ZONE_DATA, 0);
}

bool atecc_lock_status_get(uint8_t zone, bool *is_locked)
{
    // build an read command
    ATECC_PACKET packet;
    packet.param1 = 0x00;
    packet.param2 = 0x15;

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = atecc_command_read(command, &packet);
    if (status == false) {
        return false;
    }

    status = atecc_command_execute(&packet, atecc_device_ref());
    if (status == false) {
        return false;
    }

    switch (zone) {
        case LOCK_ZONE_DATA:
            *is_locked = (packet.data[ATECC_IDX_RSP_DATA + 2] == 0);
            break;
        case LOCK_ZONE_CONFIG:
            *is_locked = (packet.data[ATECC_IDX_RSP_DATA + 3] == 0);
            break;
        default:
            return false;
    }

    return true;
}

//
// ランダマイズ関連
//
bool atecc_random(uint8_t *rand_out)
{
    // build an random command
    ATECC_PACKET packet;
    packet.param1 = RANDOM_SEED_UPDATE;
    packet.param2 = 0x0000;

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_random(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }
    if (packet.data[ATECC_IDX_COUNT] != RANDOM_RSP_SIZE) {
        fido_log_error("atecc_random failed: RX_FAIL");
        return false;
    }
    if (rand_out != NULL) {
        memcpy(rand_out, &packet.data[ATECC_IDX_RSP_DATA], RANDOM_NUM_SIZE);
    }
    return true;
}