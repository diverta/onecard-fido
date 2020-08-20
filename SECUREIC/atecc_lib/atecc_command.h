/* 
 * File:   atecc_command.h
 * Author: makmorit
 *
 * Created on 2020/08/11, 12:02
 */
#ifndef ATECC_COMMAND_H
#define ATECC_COMMAND_H

#include "atecc_device.h"

#ifdef __cplusplus
extern "C" {
#endif

//
// コマンドコード関連定義
//
#define ATECC_OP_CHECKMAC       ((uint8_t)0x28)
#define ATECC_OP_DERIVE_KEY     ((uint8_t)0x1C)
#define ATECC_OP_INFO           ((uint8_t)0x30)
#define ATECC_OP_GENDIG         ((uint8_t)0x15)
#define ATECC_OP_GENKEY         ((uint8_t)0x40)
#define ATECC_OP_HMAC           ((uint8_t)0x11)
#define ATECC_OP_LOCK           ((uint8_t)0x17)
#define ATECC_OP_MAC            ((uint8_t)0x08)
#define ATECC_OP_NONCE          ((uint8_t)0x16)
#define ATECC_OP_PAUSE          ((uint8_t)0x01)
#define ATECC_OP_PRIVWRITE      ((uint8_t)0x46)
#define ATECC_OP_RANDOM         ((uint8_t)0x1B)
#define ATECC_OP_READ           ((uint8_t)0x02)
#define ATECC_OP_SIGN           ((uint8_t)0x41)
#define ATECC_OP_UPDATE_EXTRA   ((uint8_t)0x20)
#define ATECC_OP_VERIFY         ((uint8_t)0x45)
#define ATECC_OP_WRITE          ((uint8_t)0x12)
#define ATECC_OP_ECDH           ((uint8_t)0x43)
#define ATECC_OP_COUNTER        ((uint8_t)0x24)
#define ATECC_OP_SHA            ((uint8_t)0x47)
#define ATECC_OP_AES            ((uint8_t)0x51)
#define ATECC_OP_KDF            ((uint8_t)0x56)
#define ATECC_OP_SECUREBOOT     ((uint8_t)0x80)
#define ATECC_OP_SELFTEST       ((uint8_t)0x77)

// Command packet Indexes
#define ATECC_IDX_COUNT         (0)                         // command packet index for count
#define ATECC_IDX_OPCODE        (1)                         // command packet index for op-code
#define ATECC_IDX_PARAM1        (2)                         // command packet index for first parameter
#define ATECC_IDX_PARAM2        (3)                         // command packet index for second parameter
#define ATECC_IDX_DATA          (5)                         // command packet index for data load
#define ATECC_IDX_RSP_DATA      (1)                         // buffer index of data in response

// minimum number of bytes in command (from count byte to second CRC byte)
#define ATECC_CMD_SIZE_MIN      ((uint8_t)7)

//
// データ／パケットサイズ関連定義
//
#define ATECC_PUB_KEY_SIZE      (64)                        // size of a p256 public key
#define ATECC_PRIV_KEY_SIZE     (32)                        // size of a p256 private key
#define ATECC_SIG_SIZE          (64)                        // size of a p256 signature
#define ATECC_KEY_SIZE          (32)                        // size of a symmetric SHA key
#define RSA2048_KEY_SIZE        (256)                       // size of a RSA private key

#define ATECC_BLOCK_SIZE        (32)                        // size of a block
#define ATECC_WORD_SIZE         (4)                         // size of a word
#define ATECC_PUB_KEY_PAD       (4)                         // size of the public key pad
#define ATECC_SERIAL_NUM_SIZE   (9)                         // number of bytes in the device serial number
#define ATECC_KEY_COUNT         (16)                        // number of keys
#define ATECC_CONFIG_SIZE       (128)                       // size of configuration zone
#define ATECC_SHA_CONFIG_SIZE   (88)                        // size of configuration zone
#define ATECC_OTP_SIZE          (64)                        // size of OTP zone

#define ATECC_DATA_SIZE         (ATECC_KEY_COUNT * ATECC_KEY_SIZE)  // size of data zone
#define ATECC_AES_GFM_SIZE      (ATECC_BLOCK_SIZE)                  // size of GFM data

#define ATECC_COUNT_SIZE        ((uint8_t)1)                        // Number of bytes in the command packet Count
#define ATECC_CRC_SIZE          ((uint8_t)2)                        // Number of bytes in the command packet CRC
#define ATECC_PACKET_OVERHEAD   (ATECC_COUNT_SIZE + ATECC_CRC_SIZE) // Number of bytes in the command packet

#define ATECC_RSP_SIZE_MIN      ((uint8_t)4)                // minimum number of bytes in response
#define ATECC_RSP_SIZE_VAL      ((uint8_t)7)                // size of response packet containing four bytes of data
#define ATECC_RSP_SIZE_4        ((uint8_t)7)                // size of response packet containing 4 bytes data
#define ATECC_RSP_SIZE_72       ((uint8_t)75)               // size of response packet containing 64 bytes data
#define ATECC_RSP_SIZE_64       ((uint8_t)67)               // size of response packet containing 64 bytes data
#define ATECC_RSP_SIZE_32       ((uint8_t)35)               // size of response packet containing 32 bytes data
#define ATECC_RSP_SIZE_16       ((uint8_t)19)               // size of response packet containing 16 bytes data
#define ATECC_RSP_SIZE_MAX      ((uint8_t)75)               // maximum size of response packet (GenKey and Verify command)

#define OUTNONCE_SIZE           (32)                        // Size of the OutNonce response expected from several commands

#define ATECC_CHIPMODE_OFFSET           (19)                // ChipMode byte offset within the configuration zone
#define ATECC_CHIPMODE_I2C_ADDRESS_FLAG ((uint8_t)0x01)     // ChipMode I2C Address in UserExtraAdd flag
#define ATECC_CHIPMODE_TTL_ENABLE_FLAG  ((uint8_t)0x02)     // ChipMode TTLenable flag
#define ATECC_CHIPMODE_WATCHDOG_MASK    ((uint8_t)0x04)     // ChipMode watchdog duration mask
#define ATECC_CHIPMODE_WATCHDOG_SHORT   ((uint8_t)0x00)     // ChipMode short watchdog (~1.3s)
#define ATECC_CHIPMODE_WATCHDOG_LONG    ((uint8_t)0x04)     // ChipMode long watchdog (~13s)
#define ATECC_CHIPMODE_CLOCK_DIV_MASK   ((uint8_t)0xF8)     // ChipMode clock divider mask
#define ATECC_CHIPMODE_CLOCK_DIV_M0     ((uint8_t)0x00)     // ChipMode clock divider M0

//
// Lock Command 関連定義
//
#define LOCK_ZONE_IDX               ATECC_IDX_PARAM1        // Lock command index for zone
#define LOCK_SUMMARY_IDX            ATECC_IDX_PARAM2        // Lock command index for summary
#define LOCK_COUNT                  ATECC_CMD_SIZE_MIN      // Lock command packet size
#define LOCK_ZONE_CONFIG            ((uint8_t)0x00)         // Lock zone is Config
#define LOCK_ZONE_DATA              ((uint8_t)0x01)         // Lock zone is OTP or Data
#define LOCK_ZONE_DATA_SLOT         ((uint8_t)0x02)         // Lock slot of Data
#define LOCK_ZONE_NO_CRC            ((uint8_t)0x80)         // Lock command: Ignore summary.
#define LOCK_ZONE_MASK              (0xBF)                  // Lock parameter 1 bits 6 are 0.
#define ATECC_UNLOCKED              (0x55)                  // Value indicating an unlocked zone
#define ATECC_LOCKED                (0x00)                  // Value indicating a locked zone
#define LOCK_RSP_SIZE               ATECC_RSP_SIZE_MIN      // Lock command response packet size

//
// Read Command 関連定義
//
#define READ_ZONE_IDX               ATECC_IDX_PARAM1        // Read command index for zone
#define READ_ADDR_IDX               ATECC_IDX_PARAM2        // Read command index for address
#define READ_COUNT                  ATECC_CMD_SIZE_MIN      // Read command packet size
#define READ_ZONE_MASK              ((uint8_t)0x83)         // Read zone bits 2 to 6 are 0.
#define READ_4_RSP_SIZE             ATECC_RSP_SIZE_VAL      // Read command response packet size when reading 4 bytes
#define READ_32_RSP_SIZE            ATECC_RSP_SIZE_32       // Read command response packet size when reading 32 bytes

//
// Write Command 関連定義
//
#define WRITE_ZONE_IDX              ATECC_IDX_PARAM1        // Write command index for zone
#define WRITE_ADDR_IDX              ATECC_IDX_PARAM2        // Write command index for address
#define WRITE_VALUE_IDX             ATECC_IDX_DATA          // Write command index for data
#define WRITE_MAC_VS_IDX            ( 9)                    // Write command index for MAC following short data
#define WRITE_MAC_VL_IDX            (37)                    // Write command index for MAC following long data
#define WRITE_MAC_SIZE              (32)                    // Write MAC size
#define WRITE_ZONE_MASK             ((uint8_t)0xC3)         // Write zone bits 2 to 5 are 0.
#define WRITE_ZONE_WITH_MAC         ((uint8_t)0x40)         // Write zone bit 6: write encrypted with MAC
#define WRITE_ZONE_OTP              ((uint8_t)1)            // Write zone id OTP
#define WRITE_ZONE_DATA             ((uint8_t)2)            // Write zone id data
#define WRITE_RSP_SIZE              ATECC_RSP_SIZE_MIN      // Write command response packet size

//
// UpdateExtra Command 関連定義
//
#define UPDATE_MODE_IDX             ATECC_IDX_PARAM1        // UpdateExtra command index for mode
#define UPDATE_VALUE_IDX            ATECC_IDX_PARAM2        // UpdateExtra command index for new value
#define UPDATE_COUNT                ATECC_CMD_SIZE_MIN      // UpdateExtra command packet size
#define UPDATE_MODE_USER_EXTRA      ((uint8_t)0x00)         // UpdateExtra mode update UserExtra (config byte 84)
#define UPDATE_MODE_SELECTOR        ((uint8_t)0x01)         // UpdateExtra mode update Selector (config byte 85)
#define UPDATE_MODE_USER_EXTRA_ADD  UPDATE_MODE_SELECTOR    // UpdateExtra mode update UserExtraAdd (config byte 85)
#define UPDATE_MODE_DEC_COUNTER     ((uint8_t)0x02)         // UpdateExtra mode: decrement counter
#define UPDATE_RSP_SIZE             ATECC_RSP_SIZE_MIN      // UpdateExtra command response packet size

//
// PrivWrite Command 関連定義
//
#define PRIVWRITE_ZONE_IDX          ATECC_IDX_PARAM1        // PrivWrite command index for zone
#define PRIVWRITE_KEYID_IDX         ATECC_IDX_PARAM2        // PrivWrite command index for KeyID
#define PRIVWRITE_VALUE_IDX         ( 5)                    // PrivWrite command index for value
#define PRIVWRITE_MAC_IDX           (41)                    // PrivWrite command index for MAC
#define PRIVWRITE_COUNT             (75)                    // PrivWrite command packet size
#define PRIVWRITE_ZONE_MASK         ((uint8_t)0x40)         // PrivWrite zone bits 0 to 5 and 7 are 0.
#define PRIVWRITE_MODE_ENCRYPT      ((uint8_t)0x40)         // PrivWrite mode: encrypted
#define PRIVWRITE_RSP_SIZE          ATECC_RSP_SIZE_MIN      // PrivWrite command response packet size

//
// Random Command 関連定義
//
#define RANDOM_MODE_IDX             ATECC_IDX_PARAM1        // Random command index for mode
#define RANDOM_PARAM2_IDX           ATECC_IDX_PARAM2        // Random command index for 2. parameter
#define RANDOM_COUNT                ATECC_CMD_SIZE_MIN      // Random command packet size
#define RANDOM_SEED_UPDATE          ((uint8_t)0x00)         // Random mode for automatic seed update
#define RANDOM_NO_SEED_UPDATE       ((uint8_t)0x01)         // Random mode for no seed update
#define RANDOM_NUM_SIZE             ((uint8_t)32)           // Number of bytes in the data packet of a random command
#define RANDOM_RSP_SIZE             ATECC_RSP_SIZE_32       // Random command response packet size

//
// Nonce Command 関連定義
//
#define NONCE_MODE_IDX                  ATECC_IDX_PARAM1    // Nonce command index for mode
#define NONCE_PARAM2_IDX                ATECC_IDX_PARAM2    // Nonce command index for 2. parameter
#define NONCE_INPUT_IDX                 ATECC_IDX_DATA      // Nonce command index for input data
#define NONCE_COUNT_SHORT               (ATECC_CMD_SIZE_MIN + 20) // Nonce command packet size for 20 bytes of NumIn
#define NONCE_COUNT_LONG                (ATECC_CMD_SIZE_MIN + 32) // Nonce command packet size for 32 bytes of NumIn
#define NONCE_COUNT_LONG_64             (ATECC_CMD_SIZE_MIN + 64) // Nonce command packet size for 64 bytes of NumIn
#define NONCE_MODE_MASK                 ((uint8_t)0x03)     // Nonce mode bits 2 to 7 are 0.
#define NONCE_MODE_SEED_UPDATE          ((uint8_t)0x00)     // Nonce mode: update seed
#define NONCE_MODE_NO_SEED_UPDATE       ((uint8_t)0x01)     // Nonce mode: do not update seed
#define NONCE_MODE_INVALID              ((uint8_t)0x02)     // Nonce mode 2 is invalid.
#define NONCE_MODE_PASSTHROUGH          ((uint8_t)0x03)     // Nonce mode: pass-through
#define NONCE_MODE_INPUT_LEN_MASK       ((uint8_t)0x20)     // Nonce mode: input size mask
#define NONCE_MODE_INPUT_LEN_32         ((uint8_t)0x00)     // Nonce mode: input size is 32 bytes
#define NONCE_MODE_INPUT_LEN_64         ((uint8_t)0x20)     // Nonce mode: input size is 64 bytes
#define NONCE_MODE_TARGET_MASK          ((uint8_t)0xC0)     // Nonce mode: target mask
#define NONCE_MODE_TARGET_TEMPKEY       ((uint8_t)0x00)     // Nonce mode: target is TempKey
#define NONCE_MODE_TARGET_MSGDIGBUF     ((uint8_t)0x40)     // Nonce mode: target is Message Digest Buffer
#define NONCE_MODE_TARGET_ALTKEYBUF     ((uint8_t)0x80)     // Nonce mode: target is Alternate Key Buffer
#define NONCE_ZERO_CALC_MASK            ((uint16_t)0x8000)  // Nonce zero (param2): calculation mode mask
#define NONCE_ZERO_CALC_RANDOM          ((uint16_t)0x0000)  // Nonce zero (param2): calculation mode random, use RNG in calculation and return RNG output
#define NONCE_ZERO_CALC_TEMPKEY         ((uint16_t)0x8000)  // Nonce zero (param2): calculation mode TempKey, use TempKey in calculation and return new TempKey value
#define NONCE_NUMIN_SIZE                (20)                // Nonce NumIn size for random modes
#define NONCE_NUMIN_SIZE_PASSTHROUGH    (32)                // Nonce NumIn size for 32-byte pass-through mode
#define NONCE_RSP_SIZE_SHORT            ATECC_RSP_SIZE_MIN  // Nonce command response packet size with no output
#define NONCE_RSP_SIZE_LONG             ATECC_RSP_SIZE_32   // Nonce command response packet size with output

//
// GenDig Command 関連定義
//
#define GENDIG_ZONE_IDX                 ATECC_IDX_PARAM1    // GenDig command index for zone
#define GENDIG_KEYID_IDX                ATECC_IDX_PARAM2    // GenDig command index for key id
#define GENDIG_DATA_IDX                 ATECC_IDX_DATA      // GenDig command index for optional data
#define GENDIG_COUNT                    ATECC_CMD_SIZE_MIN  // GenDig command packet size without "other data"
#define GENDIG_ZONE_CONFIG              ((uint8_t)0)        // GenDig zone id config. Use KeyID to specify any of the four 256-bit blocks of the Configuration zone.
#define GENDIG_ZONE_OTP                 ((uint8_t)1)        // GenDig zone id OTP. Use KeyID to specify either the first or second 256-bit block of the OTP zone.
#define GENDIG_ZONE_DATA                ((uint8_t)2)        // GenDig zone id data. Use KeyID to specify a slot in the Data zone or a transport key in the hardware array.
#define GENDIG_ZONE_SHARED_NONCE        ((uint8_t)3)        // GenDig zone id shared nonce. KeyID specifies the location of the input value in the message generation.
#define GENDIG_ZONE_COUNTER             ((uint8_t)4)        // GenDig zone id counter. KeyID specifies the monotonic counter ID to be included in the message generation.
#define GENDIG_ZONE_KEY_CONFIG          ((uint8_t)5)        // GenDig zone id key config. KeyID specifies the slot for which the configuration information is to be included in the message generation.
#define GENDIG_RSP_SIZE                 ATECC_RSP_SIZE_MIN  // GenDig command response packet size

//
// MAC Command 関連定義
//
#define MAC_MODE_IDX                    ATECC_IDX_PARAM1    // MAC command index for mode
#define MAC_KEYID_IDX                   ATECC_IDX_PARAM2    // MAC command index for key id
#define MAC_CHALLENGE_IDX               ATECC_IDX_DATA      // MAC command index for optional challenge
#define MAC_COUNT_SHORT                 ATECC_CMD_SIZE_MIN  // MAC command packet size without challenge
#define MAC_COUNT_LONG                  (39)                // MAC command packet size with challenge
#define MAC_MODE_CHALLENGE              ((uint8_t)0x00)     // MAC mode       0: first SHA block from data slot
#define MAC_MODE_BLOCK2_TEMPKEY         ((uint8_t)0x01)     // MAC mode bit   0: second SHA block from TempKey
#define MAC_MODE_BLOCK1_TEMPKEY         ((uint8_t)0x02)     // MAC mode bit   1: first SHA block from TempKey
#define MAC_MODE_SOURCE_FLAG_MATCH      ((uint8_t)0x04)     // MAC mode bit   2: match TempKey.SourceFlag
#define MAC_MODE_PTNONCE_TEMPKEY        ((uint8_t)0x06)     // MAC mode bit   0: second SHA block from TempKey
#define MAC_MODE_PASSTHROUGH            ((uint8_t)0x07)     // MAC mode bit 0-2: pass-through mode
#define MAC_MODE_INCLUDE_OTP_88         ((uint8_t)0x10)     // MAC mode bit   4: include first 88 OTP bits
#define MAC_MODE_INCLUDE_OTP_64         ((uint8_t)0x20)     // MAC mode bit   5: include first 64 OTP bits
#define MAC_MODE_INCLUDE_SN             ((uint8_t)0x40)     // MAC mode bit   6: include serial number
#define MAC_CHALLENGE_SIZE              (32)                // MAC size of challenge
#define MAC_SIZE                        (32)                // MAC size of response
#define MAC_MODE_MASK                   ((uint8_t)0x77)     // MAC mode bits 3 and 7 are 0.
#define MAC_RSP_SIZE                    ATECC_RSP_SIZE_32   // MAC command response packet size

//
// GenKey Command 関連定義
//
#define GENKEY_MODE_IDX                 ATECC_IDX_PARAM1    // GenKey command index for mode
#define GENKEY_KEYID_IDX                ATECC_IDX_PARAM2    // GenKey command index for key id
#define GENKEY_DATA_IDX                 (5)                 // GenKey command index for other data
#define GENKEY_COUNT                    ATECC_CMD_SIZE_MIN  // GenKey command packet size without "other data"
#define GENKEY_COUNT_DATA               (10)                // GenKey command packet size with "other data"
#define GENKEY_OTHER_DATA_SIZE          (3)                 // GenKey size of "other data"
#define GENKEY_MODE_MASK                ((uint8_t)0x1C)     // GenKey mode bits 0 to 1 and 5 to 7 are 0
#define GENKEY_MODE_PRIVATE             ((uint8_t)0x04)     // GenKey mode: private key generation
#define GENKEY_MODE_PUBLIC              ((uint8_t)0x00)     // GenKey mode: public key calculation
#define GENKEY_MODE_DIGEST              ((uint8_t)0x08)     // GenKey mode: PubKey digest will be created after the public key is calculated
#define GENKEY_MODE_PUBKEY_DIGEST       ((uint8_t)0x10)     // GenKey mode: Calculate PubKey digest on the public key in KeyId
#define GENKEY_PRIVATE_TO_TEMPKEY       ((uint16_t)0xFFFF)  // GenKey Create private key and store to tempkey (608 only)
#define GENKEY_RSP_SIZE_SHORT           ATECC_RSP_SIZE_MIN  // GenKey response packet size in Digest mode
#define GENKEY_RSP_SIZE_LONG            ATECC_RSP_SIZE_64   // GenKey response packet size when returning a public key

typedef struct {
    uint8_t  opcode;
    uint16_t execution_time_msec;
} device_execution_time_t;

#pragma pack( push, ATECC_PACKET, 2 )
typedef struct {
    // used for transmit/send
    // used by HAL layer as needed (I/O tokens, Word address values)
    uint8_t _reserved;  

    //--- start of packet i/o frame----
    uint8_t  txsize;
    uint8_t  opcode;
    uint8_t  param1;    // often same as mode
    uint16_t param2;

    // includes 2-byte CRC.  
    // Data size is determined by largest possible data section 
    // of any command + crc 
    // (see: x08 verify data1 + data2 + data3 + data4)
    // this is an explicit design trade-off (space) 
    // resulting in simplicity in use and implementation
    uint8_t  data[192]; 
    //--- end of packet i/o frame

    // used for receive
    uint8_t execTime;       // execution time of command by opcode

} ATECC_PACKET;
#pragma pack( pop, ATECC_PACKET)

//
// 関数群
//
bool atecc_command_lock(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_read(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_write(ATECC_COMMAND command, ATECC_PACKET *packet, bool has_mac);
bool atecc_command_update_extra(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_execute(ATECC_PACKET* packet, ATECC_DEVICE device);
bool atecc_command_priv_write(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_random(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_nonce(ATECC_COMMAND command, ATECC_PACKET *packet);
bool atecc_command_gen_dig(ATECC_COMMAND command, ATECC_PACKET *packet, bool is_no_mac_key);
bool atecc_command_gen_key(ATECC_COMMAND command, ATECC_PACKET *packet);

#ifdef __cplusplus
}
#endif

#endif /* ATECC_COMMAND_H */
