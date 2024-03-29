/* 
 * File:   atecc_write.c
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:27
 */
#include <stdlib.h>
#include <string.h>

#include "atecc_command.h"
#include "atecc_device.h"
#include "atecc_util.h"
#include "atecc_write.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static bool atecc_write(uint8_t zone, uint16_t address, const uint8_t *value, const uint8_t *mac)
{
    if (value == NULL) {
        fido_log_error("atecc_write failed: BAD_PARAM");
        return false;
    }

    // Build the write command
    ATECC_PACKET packet;
    packet.param1 = zone;
    packet.param2 = address;
    if (zone & ATECC_ZONE_READWRITE_32) {
        // 32-byte write
        memcpy(packet.data, value, ATECC_BLOCK_SIZE);
        // Only 32-byte writes can have a MAC
        if (mac) {
            memcpy(&packet.data[ATECC_BLOCK_SIZE], mac, WRITE_MAC_SIZE);
        }

    } else {
        // 4-byte write
        memcpy(packet.data, value, ATECC_WORD_SIZE);
    }
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = atecc_command_write(command, &packet, mac && (zone & ATECC_ZONE_READWRITE_32));
    if (status == false) {
        return status;
    }

    // Execute the write command
    status = atecc_command_execute(&packet, atecc_device_ref());
    if (status == false) {
        return status;
    }

    return true;
}

bool atecc_write_zone(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, const uint8_t *data, uint8_t len)
{
    // Check the input parameters
    if (data == NULL) {
        fido_log_error("atecc_write_zone failed: BAD_PARAM");
        return false;
    }

    if (len != ATECC_WORD_SIZE && len != ATECC_BLOCK_SIZE) {
        fido_log_error("atecc_write_zone failed: BAD_PARAM");
        return false;
    }

    // The get address function checks the remaining variables
    uint16_t addr;
    bool status = atecc_get_address(zone, slot, block, offset, &addr);
    if (status == false) {
        return status;
    }

    // If there are 32 bytes to write, then xor the bit into the mode
    if (len == ATECC_BLOCK_SIZE) {
        zone = zone | ATECC_ZONE_READWRITE_32;
    }

    status = atecc_write(zone, addr, data, NULL);
    return status;
}

bool atecc_write_bytes_zone(uint8_t zone, uint16_t slot, size_t offset_bytes, const uint8_t *data, size_t length)
{
    if (zone != ATECC_ZONE_CONFIG && zone != ATECC_ZONE_OTP && zone != ATECC_ZONE_DATA) {
        fido_log_error("atecc_write_bytes_zone failed: BAD_PARAM");
        return false;
    }
    if (zone == ATECC_ZONE_DATA && slot > 15) {
        fido_log_error("atecc_write_bytes_zone failed: BAD_PARAM");
        return false;
    }
    if (length == 0) {
        // Always succeed writing 0 bytes
        return true;
    }
    if (data == NULL) {
        fido_log_error("atecc_write_bytes_zone failed: BAD_PARAM");
        return false;
    }
    if (offset_bytes % ATECC_WORD_SIZE != 0 || length % ATECC_WORD_SIZE != 0) {
        fido_log_error("atecc_write_bytes_zone failed: BAD_PARAM");
        return false;
    }

    size_t zone_size = 0;
    bool status = atecc_get_zone_size(zone, slot, &zone_size);
    if (status == false) {
        return status;
    }
    if (offset_bytes + length > zone_size) {
        fido_log_error("atecc_write_bytes_zone failed: BAD_PARAM");
        return false;
    }

    size_t data_idx = 0;
    size_t cur_block = offset_bytes / ATECC_BLOCK_SIZE;
    size_t cur_word = (offset_bytes % ATECC_BLOCK_SIZE) / ATECC_WORD_SIZE;

    while (data_idx < length) {
        // The last item makes sure we handle the selector, user extra, 
        // and lock bytes in the config properly
        if (cur_word == 0 && length - data_idx >= ATECC_BLOCK_SIZE && !(zone == ATECC_ZONE_CONFIG && cur_block == 2)) {
            status = atecc_write_zone(zone, slot, (uint8_t)cur_block, 0, &data[data_idx], ATECC_BLOCK_SIZE);
            if (status == false) {
                return status;
            }
            data_idx += ATECC_BLOCK_SIZE;
            cur_block += 1;

        } else {
            // Skip trying to change UserExtra, Selector, LockValue, and LockConfig 
            // which require the UpdateExtra command to change
            if (!(zone == ATECC_ZONE_CONFIG && cur_block == 2 && cur_word == 5)) {
                status = atecc_write_zone(zone, slot, (uint8_t)cur_block, (uint8_t)cur_word, &data[data_idx], ATECC_WORD_SIZE);
                if (status == false) {
                    return status;
                }
            }
            data_idx += ATECC_WORD_SIZE;
            cur_word += 1;
            if (cur_word == ATECC_BLOCK_SIZE / ATECC_WORD_SIZE) {
                cur_block += 1;
                cur_word = 0;
            }
        }
    }

    return true;
}

static bool atecc_update_extra(uint8_t mode, uint16_t new_value)
{
    // Build command
    ATECC_PACKET packet;
    memset(&packet, 0, sizeof(packet));
    packet.param1 = mode;
    packet.param2 = new_value;
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = atecc_command_update_extra(command, &packet);
    if (status == false) {
        return status;
    }

    // Execute command
    status = atecc_command_execute(&packet, atecc_device_ref());
    if (status == false) {
        return status;
    }

    return true;
}

bool atecc_write_config_zone(const uint8_t *config_data)
{
    if (config_data == NULL) {
        fido_log_error("atecc_write_config_zone failed: BAD_PARAM");
        return false;
    }

    // Get config zone size for the device
    size_t config_size = 0;
    bool status = atecc_get_zone_size(ATECC_ZONE_CONFIG, 0, &config_size);
    if (status == false) {
        return status;
    }

    // Write config zone excluding UserExtra and Selector
    status = atecc_write_bytes_zone(ATECC_ZONE_CONFIG, 0, 16, &config_data[16], config_size - 16);
    if (status == false) {
        return status;
    }

    // Write the UserExtra and Selector. 
    // This may fail if either value is already non-zero.
    status = atecc_update_extra(UPDATE_MODE_USER_EXTRA, config_data[84]);
    if (status == false) {
        return status;
    }
    status = atecc_update_extra(UPDATE_MODE_SELECTOR, config_data[85]);
    if (status == false) {
        return status;
    }

    return true;
}
