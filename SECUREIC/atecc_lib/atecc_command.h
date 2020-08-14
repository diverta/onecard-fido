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

#ifdef __cplusplus
}
#endif

#endif /* ATECC_COMMAND_H */
