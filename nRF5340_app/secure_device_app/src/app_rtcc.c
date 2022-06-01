/* 
 * File:   app_rtcc.c
 * Author: makmorit
 *
 * Created on 2022/06/01, 12:08
 */
#include <stdio.h>
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/i2c.h>

#include "app_rtcc_define.h"

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_rtcc);

static const struct device *i2c_dev;

// データ送受信用の一時領域
static struct i2c_msg msgs[2];
static uint8_t read_buff[32];
static uint8_t write_buff[32];

//
// I2C write & read
//
static bool read_register(uint8_t reg_addr, uint8_t *reg_val)
{
    write_buff[0] = reg_addr;

    // Send the address to read from
    msgs[0].buf = write_buff;
    msgs[0].len = 1U;
    msgs[0].flags = I2C_MSG_WRITE;

    // Read from device. STOP after this
    msgs[1].buf = read_buff;
    msgs[1].len = 1U;
    msgs[1].flags = I2C_MSG_READ | I2C_MSG_STOP;

    if (i2c_transfer(i2c_dev, &msgs[0], 2, RV3028C7_ADDRESS) != 0) {
        LOG_DBG("i2c_transfer error");
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
    msgs[0].buf = write_buff;
    msgs[0].len = 2U;
    msgs[0].flags = I2C_MSG_WRITE | I2C_MSG_STOP;

    if (i2c_transfer(i2c_dev, &msgs[0], 2, RV3028C7_ADDRESS) != 0) {
        LOG_DBG("i2c_transfer error");
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
        k_sleep(K_MSEC(10));
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

static bool set_backup_switchover_mode(uint8_t val)
{
    if (val > 3) {
        return false;
    }

    // Read EEPROM Backup Register (0x37)
    uint8_t backup_reg_val;
    if (read_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, &backup_reg_val) == false) {
        LOG_ERR("Read EEPROM backup register fail");
        return false;
    }
    if (backup_reg_val == 0xFF) {
        LOG_ERR("Invalid EEPROM backup register value");
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
        LOG_ERR("Write EEPROM backup register fail");
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
        LOG_ERR("Read EEPROM backup register fail");
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
        LOG_ERR("Write EEPROM backup register fail");
        return false;
    }
    return true;
}

//
// RTCCの初期化
//
bool app_rtcc_initialize(void)
{
    // 制御レジスター（Control 2 register）を参照、0x00なら正常
    uint8_t c2;
    if (read_register(RV3028C7_REG_CONTROL_2, &c2) == false) {
        return false;
    }
    if (c2 != 0x00) {
        LOG_ERR("RTCC is not available");
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
    uint8_t val = 0x03;
    if (set_backup_switchover_mode(val) == false) {
        LOG_ERR("RTCC backup switchover mode setting failed");
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
        LOG_ERR("RTCC tricle charge setting failed");
        return false;
    }

    return true;
}

//
// デバイスの初期化
//
static int app_rtcc_init(const struct device *dev)
{
    // I2C（i2c1）デバイス初期化
    (void)dev;
    i2c_dev = DEVICE_DT_GET(DT_NODELABEL(i2c1));
    if (device_is_ready(i2c_dev) == false) {
        LOG_ERR("RTCC device is not ready");
        return -ENOTSUP;
    }

    LOG_INF("RTCC device is ready");
    return 0;
}

SYS_INIT(app_rtcc_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
