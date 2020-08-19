/* 
 * File:   atecc_read.c
 * Author: makmorit
 *
 * Created on 2020/08/11, 11:26
 */
#include <string.h>

#include "atecc_command.h"
#include "atecc_device.h"
#include "atecc_iface.h"
#include "atecc_read.h"
#include "atecc_util.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

static bool atecc_read_zone(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint8_t *data, uint8_t len)
{
    ATECC_PACKET  packet;
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    bool status = false;
    uint16_t      addr;

    // Check the input parameters
    if (data == NULL) {
        fido_log_error("atecc_read_zone failed: BAD_PARAM");
        return false;
    }
    if (len != ATECC_WORD_SIZE && len != ATECC_BLOCK_SIZE) {
        fido_log_error("atecc_read_zone failed: BAD_PARAM");
        return false;
    }

    // The get address function checks the remaining variables
    status = atecc_get_address(zone, slot, block, offset, &addr);
    if (status == false) {
        return status;
    }

    // If there are 32 bytes to read, then OR the bit into the mode
    if (len == ATECC_BLOCK_SIZE) {
        zone = zone | ATECC_ZONE_READWRITE_32;
    }

    // build a read command
    packet.param1 = zone;
    packet.param2 = addr;
    status = atecc_command_read(command, &packet);
    if (status == false) {
        return status;
    }

    // execute a read command
    status = atecc_command_execute(&packet, atecc_device_ref());
    if (status == false) {
        return status;
    }

    memcpy(data, &packet.data[1], len);
    return true;
}

bool atecc_read_config_zone(uint8_t *config_data)
{
    if (config_data == NULL) {
        fido_log_error("atecc_read_config_zone failed: BAD_PARAM");
        return false;
    }

    bool status = atecc_read_bytes_zone(ATECC_ZONE_CONFIG, 0, 0x00, config_data, ATECC_CONFIG_SIZE);
    if (status == false) {
        return false;
    }
    return true;
}

bool atecc_read_bytes_zone(uint8_t zone, uint16_t slot, size_t offset, uint8_t *data, size_t length)
{
    size_t  zone_size = 0;
    uint8_t read_buf[32];
    size_t  data_idx = 0;
    size_t  cur_block = 0;
    size_t  cur_offset = 0;
    uint8_t read_size = ATECC_BLOCK_SIZE;
    size_t  read_buf_idx = 0;
    size_t  copy_length = 0;
    size_t  read_offset = 0;

    if (zone != ATECC_ZONE_CONFIG && zone != ATECC_ZONE_OTP && zone != ATECC_ZONE_DATA) {
        fido_log_error("atecc_read_bytes_zone failed: BAD_PARAM");
        return false;
    }
    if (zone == ATECC_ZONE_DATA && slot > 15) {
        fido_log_error("atecc_read_bytes_zone failed: BAD_PARAM");
        return false;
    }
    if (length == 0) {
        // Always succeed reading 0 bytes
        return true;
    }
    if (data == NULL) {
        fido_log_error("atecc_read_bytes_zone failed: BAD_PARAM");
        return false;
    }

    bool status = atecc_get_zone_size(zone, slot, &zone_size);
    if (status == false) {
        return status;
    }
    if (offset + length > zone_size) {
        fido_log_error("atecc_read_bytes_zone failed: Can't read past the end of a zone");
        return false;
    }
    cur_block = offset / ATECC_BLOCK_SIZE;

    while (data_idx < length) {
        if (read_size == ATECC_BLOCK_SIZE && zone_size - cur_block * ATECC_BLOCK_SIZE < ATECC_BLOCK_SIZE) {
            // We have less than a block to read and 
            // can't read past the end of the zone, 
            // switch to word reads
            read_size = ATECC_WORD_SIZE;
            cur_offset = ((data_idx + offset) / ATECC_WORD_SIZE) % (ATECC_BLOCK_SIZE / ATECC_WORD_SIZE);
        }
        // Read next chunk of data
        status = atecc_read_zone(zone, slot, (uint8_t)cur_block, (uint8_t)cur_offset, read_buf, read_size);
        if (status == false) {
            return status;
        }
        // Calculate where in the read buffer we need data from
        read_offset = cur_block * ATECC_BLOCK_SIZE + cur_offset * ATECC_WORD_SIZE;
        if (read_offset < offset) {
            // Read data starts before the requested chunk
            read_buf_idx = offset - read_offset;
        } else {
            // Read data is within the requested chunk
            read_buf_idx = 0;

        }
        // Calculate how much data from the read buffer we want to copy
        if (length - data_idx < read_size - read_buf_idx) {
            copy_length = length - data_idx;
        } else {
            copy_length = read_size - read_buf_idx;
        }

        memcpy(&data[data_idx], &read_buf[read_buf_idx], copy_length);
        data_idx += copy_length;
        if (read_size == ATECC_BLOCK_SIZE) {
            cur_block += 1;
        } else {
            cur_offset += 1;
        }
    }

    return true;
}

bool atecc_read_serial_number(uint8_t *serial_number)
{
    if (serial_number == NULL) {
        fido_log_error("atecc_read_bytes_zone failed: BAD_PARAM");
        return false;
    }

    uint8_t read_buf[ATECC_BLOCK_SIZE];
    bool status = atecc_read_zone(ATECC_ZONE_CONFIG, 0, 0, 0, read_buf, ATECC_BLOCK_SIZE);
    if (status == false) {
        return status;
    }

    memcpy(&serial_number[0], &read_buf[0], 4);
    memcpy(&serial_number[4], &read_buf[8], 5);
    return true;
}
