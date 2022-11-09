/* 
 * File:   rv3028c7_i2c.c
 * Author: makmorit
 *
 * Created on 2022/11/09, 15:43
 */
#include <stdbool.h>

// for logging informations
#define NRF_LOG_MODULE_NAME rv3028c7_i2c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for nrf_delay_ms
#include "nrf_delay.h"

// for I2C access
#include "fido_twi.h"

// for defines
#include "rv3028c7_define.h"

// データ送受信用の一時領域
static uint8_t read_buff[32];
static uint8_t write_buff[32];

//
// I2C write & read
//
static bool read_register(uint8_t reg_addr, uint8_t *reg_val)
{
    write_buff[0] = reg_addr;

    // Send the address to read from
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 1) == false) {
        return false;
    }

    // Read from device. STOP after this
    if (fido_twi_read(RV3028C7_ADDRESS, read_buff, 1) == false) {
        return false;
    }

    *reg_val = read_buff[0];
    return true;
}

static bool write_register(uint8_t reg_addr, uint8_t reg_val)
{
    write_buff[0] = reg_addr;
    write_buff[1] = reg_val;

    // Write to device. STOP after this
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 2) == false) {
        return false;
    }

    return true;
}

static bool wait_for_eeprom()
{
    uint8_t reg_val;
    for (uint8_t c = 0; c < 10; c++) {
        // ステータスレジスターの値を取得
        if (read_register(RV3028C7_REG_STATUS, &reg_val) == false) {
            return false;
        }
        if ((reg_val & (1 << RV3028C7_BIT_STATUS_EEBUSY)) == 0) {
            // ステータスがBUSYでなければ終了
            return true;
        }
        // 10ms wait
        nrf_delay_ms(10);
    }
    // タイムアウトの場合はfalse
    return false;
}

static bool disable_auto_refresh_with_eerd_bit(uint8_t *ctr1_reg_val)
{
    // Disable auto refresh by writing 1 to EERD control bit in CTRL1 register
    if (read_register(RV3028C7_REG_CONTROL_1, ctr1_reg_val) == false) {
        return false;
    }
    *ctr1_reg_val |= (1 << RV3028C7_BIT_CTRL1_EERD);
    if (write_register(RV3028C7_REG_CONTROL_1, *ctr1_reg_val) == false) {
        return false;
    }
    return true;
}

static bool reenable_auto_refresh_with_eerd_bit(uint8_t *ctr1_reg_val)
{
    // Reenable auto refresh by writing 0 to EERD control bit in CTRL1 register
    if (read_register(RV3028C7_REG_CONTROL_1, ctr1_reg_val) == false) {
        return false;
    }
    if (*ctr1_reg_val == 0x00) {
        return false;
    }
    *ctr1_reg_val &= ~(1 << RV3028C7_BIT_CTRL1_EERD);
    if (write_register(RV3028C7_REG_CONTROL_1, *ctr1_reg_val) == false) {
        return false;
    }
    return true;
}

bool read_eeprom_backup_register(uint8_t reg_addr, uint8_t *reg_val)
{
    if (wait_for_eeprom() == false) {
        return false;
    }

    // Disable auto refresh by writing 1 to EERD control bit in CTRL1 register
    uint8_t ctrl1;
    if (disable_auto_refresh_with_eerd_bit(&ctrl1) == false) {
        return false;
    }

    // Read EEPROM Register
    if (write_register(RV3028C7_REG_EEPROM_ADDR, reg_addr) == false) {
        return false;
    }
    if (write_register(RV3028C7_REG_EEPROM_CMD, RV3028C7_CMD_EEPROM_FIRST) == false) {
        return false;
    }
    if (write_register(RV3028C7_REG_EEPROM_CMD, RV3028C7_CMD_EEPROM_READ_SINGLE) == false) {
        return false;
    }
    if (wait_for_eeprom() == false) {
        return false;
    }
    if (read_register(RV3028C7_REG_EEPROM_DATA, reg_val) == false) {
        return false;
    }
    if (wait_for_eeprom() == false) {
        return false;
    }

    // Reenable auto refresh by writing 0 to EERD control bit in CTRL1 register
    if (reenable_auto_refresh_with_eerd_bit(&ctrl1) == false) {
        return false;
    }

    return true;
}

static bool write_eeprom_backup_register(uint8_t reg_addr, uint8_t reg_val)
{
    if (wait_for_eeprom() == false) {
        return false;
    }

    // Disable auto refresh by writing 1 to EERD control bit in CTRL1 register
    uint8_t ctrl1;
    if (disable_auto_refresh_with_eerd_bit(&ctrl1) == false) {
        return false;
    }

    // Write Configuration RAM Register
    if (write_register(reg_addr, reg_val) == false) {
        return false;
    }

    // Update EEPROM (All Configuration RAM -> EEPROM)
    if (write_register(RV3028C7_REG_EEPROM_CMD, RV3028C7_CMD_EEPROM_FIRST) == false) {
        return false;
    }
    if (write_register(RV3028C7_REG_EEPROM_CMD, RV3028C7_CMD_EEPROM_UPDATE) == false) {
        return false;
    }
    if (wait_for_eeprom() == false) {
        return false;
    }

    // Reenable auto refresh by writing 0 to EERD control bit in CTRL1 register
    if (reenable_auto_refresh_with_eerd_bit(&ctrl1) == false) {
        return false;
    }
    if (wait_for_eeprom() == false) {
        return false;
    }

    return true;
}
//
// Backup settings
//
static bool set_backup_switchover_mode(uint8_t val)
{
    if (val > 3) {
        return false;
    }

    // Read EEPROM Backup Register (0x37)
    uint8_t backup_reg_val;
    if (read_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, &backup_reg_val) == false) {
        NRF_LOG_ERROR("Read EEPROM backup register fail");
        return false;
    }
    if (backup_reg_val == 0xFF) {
        NRF_LOG_ERROR("Invalid EEPROM backup register value");
        return false;
    }

    // Ensure FEDE Bit is set to 1
    backup_reg_val |= (1 << RV3028C7_BIT_EEPROM_BACKUP_FEDE);
    // Set BSM Bits (Backup Switchover Mode)
    //  Clear BSM Bits of EEPROM Backup Register
    backup_reg_val &= RV3028C7_MASK_EEPROM_BACKUP_BSM_CLEAR;
    //  Shift values into EEPROM Backup Register
    backup_reg_val |= (val << RV3028C7_SHIFT_EEPROM_BACKUP_BSM);

    // Write EEPROM Backup Register
    if (write_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, backup_reg_val) == false) {
        NRF_LOG_ERROR("Write EEPROM backup register fail");
        return false;
    }
    return true;
}

static bool enable_trickle_charge(bool enable, uint8_t tcr)
{
    if (tcr > 3) {
        return false;
    }

    // Read EEPROM Backup Register (0x37)
    uint8_t backup_reg_val;
    if (read_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, &backup_reg_val) == false) {
        NRF_LOG_ERROR("Read EEPROM backup register fail");
        return false;
    }

    // Clear TCE Bit (Trickle Charge Enable)
    backup_reg_val &= RV3028C7_MASK_EEPROM_BACKUP_TCE_CLEAR;

    // Clear TCR Bits (Trickle Charge Resistor)
    backup_reg_val &= RV3028C7_MASK_EEPROM_BACKUP_TCR_CLEAR;

    if (enable) {
        // Set TCR Bits (Trickle Charge Resistor)
        //  Shift values into EEPROM Backup Register
        backup_reg_val |= (tcr << RV3028C7_SHIFT_EEPROM_BACKUP_TCR);
        // Write 1 to TCE Bit
        backup_reg_val |= (1 << RV3028C7_BIT_EEPROM_BACKUP_TCE);
    }

    // Write EEPROM Backup Register
    if (write_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, backup_reg_val) == false) {
        NRF_LOG_ERROR("Write EEPROM backup register fail");
        return false;
    }

    NRF_LOG_DEBUG("Write EEPROM backup register success (0x%02x)", backup_reg_val);
    return true;
}

//
// RTCCの初期化
//
bool rv3028c7_initialize(void)
{
    // デバイスの初期化
    if (fido_twi_init() == false) {
        NRF_LOG_DEBUG("rv3028c7_initialize failed: Communication with device failed. Same as in hardware dependent modules.");
        return false;
    }

    // 制御レジスター（Control 2 register）を参照、0x00なら正常
    uint8_t c2;
    if (read_register(RV3028C7_REG_CONTROL_2, &c2) == false) {
        return false;
    }
    if (c2 != 0x00) {
        NRF_LOG_DEBUG("RTCC is not available");
        return false;
    }

    //
    // 設定時刻の永続化のため、
    // VDD_NRFからの電源供給がなくなった場合、
    // 自動的に外部バックアップ電源に切り替えるよう設定
    //
    // 0 = Switchover disabled
    // 1 = Direct Switching Mode
    // 2 = Standby Mode
    // 3 = Level Switching Mode
    uint8_t val = 0x00;
    if (set_backup_switchover_mode(val) == false) {
        NRF_LOG_DEBUG("RTCC backup switchover mode setting failed");
        return false;
    }

    // 
    // トリクル充電は行わないよう設定。
    // TODO:
    //   将来的にトリクル充電を行わせる場合、
    //   外部提供電源のインピーダンスを設定
    //   0 =  3kOhm
    //   1 =  5kOhm
    //   2 =  9kOhm
    //   3 = 15kOhm
    //
    uint8_t tcr = 0x03;
    if (enable_trickle_charge(false, tcr) == false) {
        NRF_LOG_DEBUG("RTCC tricle charge setting failed");
        return false;
    }

    return true;
}
