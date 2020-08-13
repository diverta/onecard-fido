/* 
 * File:   atecc_util.c
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:36
 */
#include <stdlib.h>

#include "atecc_command.h"
#include "atecc_util.h"

ATECC_STATUS atecc_get_address(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t* addr)
{
    uint8_t mem_zone = zone & 0x03;

    if (addr == NULL) {
        return ATECC_BAD_PARAM;
    }
    if ((mem_zone != ATECC_ZONE_CONFIG) && (mem_zone != ATECC_ZONE_DATA) && (mem_zone != ATECC_ZONE_OTP)) {
        return ATECC_BAD_PARAM;
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

    return ATECC_SUCCESS;
}

ATECC_STATUS atecc_get_zone_size(uint8_t zone, uint16_t slot, size_t* size)
{
    ATECC_STATUS status = ATECC_SUCCESS;

    if (size == NULL) {
        return ATECC_BAD_PARAM;
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
                status = ATECC_BAD_PARAM;
            }
            break;
        default:
            status = ATECC_BAD_PARAM;
            break;
    }

    return status;
}
