/* 
 * File:   app_rtcc_define.h
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:16
 */
#ifndef APP_RTCC_DEFINE_H
#define APP_RTCC_DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

// I2Cスレーブアドレス
#define RV3028C7_ADDRESS                0x52

// レジスターアドレス
#define RV3028C7_REG_STATUS             0x0e
#define RV3028C7_REG_CONTROL_1          0x0f
#define RV3028C7_REG_CONTROL_2          0x10
#define RV3028C7_REG_CLOCK_SECONDS      0x00
#define RV3028C7_REG_EEPROM_ADDR        0x25
#define RV3028C7_REG_EEPROM_DATA        0x26
#define RV3028C7_REG_EEPROM_CMD         0x27
#define RV3028C7_REG_EEPROM_CLKOUT      0x35
#define RV3028C7_REG_EEPROM_BACKUP      0x37

// Bits in Status Register
#define RV3028C7_BIT_STATUS_EEBUSY      7
#define RV3028C7_BIT_STATUS_CLKF        6
#define RV3028C7_BIT_STATUS_BSF         5
#define RV3028C7_BIT_STATUS_UF          4
#define RV3028C7_BIT_STATUS_TF          3
#define RV3028C7_BIT_STATUS_AF          2
#define RV3028C7_BIT_STATUS_EVF         1
#define RV3028C7_BIT_STATUS_PORF        0

// Bits in Control1 Register
#define RV3028C7_BIT_CTRL1_TRPT         7
#define RV3028C7_BIT_CTRL1_WADA         5
#define RV3028C7_BIT_CTRL1_USEL         4
#define RV3028C7_BIT_CTRL1_EERD         3
#define RV3028C7_BIT_CTRL1_TE           2
#define RV3028C7_BIT_CTRL1_TD1          1
#define RV3028C7_BIT_CTRL1_TD0          0

// Commands for EEPROM Command Register
#define RV3028C7_CMD_EEPROM_FIRST               0x00
#define RV3028C7_CMD_EEPROM_UPDATE              0x11
#define RV3028C7_CMD_EEPROM_REFRESH             0x12
#define RV3028C7_CMD_EEPROM_WRITE_SINGLE        0x21
#define RV3028C7_CMD_EEPROM_READ_SINGLE         0x22

// Bits in EEPROM Backup Register
#define RV3028C7_BIT_EEPROM_BACKUP_TCE          5       // Trickle Charge Enable Bit
#define RV3028C7_BIT_EEPROM_BACKUP_FEDE         4       // Fast Edge Detection Enable Bit (for Backup Switchover Mode)
#define RV3028C7_SHIFT_EEPROM_BACKUP_BSM        2       // Backup Switchover Mode shift
#define RV3028C7_SHIFT_EEPROM_BACKUP_TCR        0       // Trickle Charge Resistor shift
#define RV3028C7_MASK_EEPROM_BACKUP_BSM_CLEAR   0xf3    // 0b11110011 = Backup Switchover Mode clear
#define RV3028C7_MASK_EEPROM_BACKUP_TCR_CLEAR   0xfc    // 0b11111100 = Trickle Charge Resistor clear
#define RV3028C7_MASK_EEPROM_BACKUP_TCE_CLEAR   0xdf    // 0b11011111 = Trickle Charge Enable Bit clear

#ifdef __cplusplus
}
#endif

#endif /* APP_RTCC_DEFINE_H */
