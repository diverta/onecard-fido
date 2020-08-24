/* 
 * File:   atecc_util.h
 * Author: makmorit
 *
 * Created on 2020/08/12, 12:36
 */
#ifndef ATECC_UTIL_H
#define ATECC_UTIL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// 各種構造体定義
//
typedef struct {
    uint8_t  value[64];                     // Value of TempKey (64 bytes for ATECC608A only)
    unsigned key_id       : 4;              // If TempKey was derived from a slot or transport key (GenDig or GenKey), that key ID is saved here.
    unsigned source_flag  : 1;              // Indicates id TempKey started from a random nonce (0) or not (1).
    unsigned gen_dig_data : 1;              // TempKey was derived from the GenDig command.
    unsigned gen_key_data : 1;              // TempKey was derived from the GenKey command (ATECC devices only).
    unsigned no_mac_flag  : 1;              // TempKey was derived from a key that has the NoMac bit set preventing the use of the MAC command. Known as CheckFlag in ATSHA devices).
    unsigned valid        : 1;              // TempKey is valid.
    uint8_t  is_64;                         // TempKey has 64 bytes of valid data
} atecc_temp_key_t;

typedef struct {
    uint8_t               mode;
    uint16_t              zero;
    const uint8_t        *num_in;
    const uint8_t        *rand_out;
    atecc_temp_key_t     *temp_key;
} atecc_nonce_in_out_t;

typedef struct {
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
    atecc_temp_key_t     *temp_key;         // [inout] Current state of TempKey
} atecc_gen_dig_in_out_t;

typedef struct {
    uint8_t               zone;             // Zone/Param1 for the Write or PrivWrite command
    uint16_t              key_id;           // KeyID/Param2 for the Write or PrivWrite command
    const uint8_t        *sn;               // Device serial number SN[0:8]. Only SN[0:1] and SN[8] are required though.
    const uint8_t        *input_data;       // Data to be encrypted. 32 bytes for Write command, 36 bytes for PrivWrite command.
    uint8_t              *encrypted_data;   // Encrypted version of input_data will be returned here. 32 bytes for Write command, 36 bytes for PrivWrite command.
    uint8_t              *auth_mac;         // Write MAC will be returned here. 32 bytes.
    atecc_temp_key_t     *temp_key;         // Current state of TempKey.
} atecc_write_mac_in_out_t;//

//
// 関数群
//
bool atecc_get_address(uint8_t zone, uint16_t slot, uint8_t block, uint8_t offset, uint16_t *addr);
bool atecc_get_zone_size(uint8_t zone, uint16_t slot, size_t *size);
bool atecc_lock_config_zone(void);
bool atecc_lock_data_zone(void);
bool atecc_lock_status_get(uint8_t zone, bool *is_locked);
bool atecc_random(uint8_t *rand_out);
bool atecc_gen_key(uint8_t mode, uint16_t key_id, const uint8_t *other_data, uint8_t *public_key);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_UTIL_H */
