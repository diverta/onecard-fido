/* 
 * File:   rv3028c7_i2c.c
 * Author: makmorit
 *
 * Created on 2022/11/09, 15:43
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// for struct tm
#include <time.h>

// for logging informations
#define NRF_LOG_MODULE_NAME rv3028c7_i2c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for fido_board_delay_ms
#include "fido_board.h"

// for I2C access
#include "fido_twi.h"

// for defines
#include "rv3028c7_define.h"

// データ送受信用の一時領域
static uint8_t read_buff[32];
static uint8_t write_buff[32];
static uint8_t m_datetime[DATETIME_COMPONENTS_SIZE];

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

static bool read_bytes_from_register(uint8_t reg_addr, uint8_t *data, uint8_t size) 
{
    write_buff[0] = reg_addr;

    // Send the address to read from
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 1) == false) {
        return false;
    }

    // Read from device. STOP after this
    if (fido_twi_read(RV3028C7_ADDRESS, read_buff, size) == false) {
        return false;
    }

    memcpy(data, read_buff, size);
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

static bool write_bytes_to_register(uint8_t reg_addr, uint8_t *data, uint8_t size) 
{
    write_buff[0] = reg_addr;
    memcpy(write_buff + 1, data, size);

    // Write to device. STOP after this
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, size + 1) == false) {
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
        fido_board_delay_ms(10);
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
// UNIX timestamping
//
static uint8_t convert_to_bcd(uint8_t decimal) 
{
    return (decimal / 10 * 16) + (decimal % 10);
}

static bool set_datetime_components(uint8_t *datetime_components, uint16_t year, uint8_t month, uint8_t day_of_month, uint8_t day_of_week, uint8_t hour, uint8_t minute, uint8_t second) 
{
    // Year 2000 AD is the earliest allowed year in this implementation
    // Century overflow is not considered yet 
    // (i.e., only supports year 2000 to 2099)
    if (year < 2000) {
        return false;
    }
    datetime_components[DATETIME_YEAR] = convert_to_bcd(year - 2000);

    if (month < 1 || month > 12) {
        return false;
    }
    datetime_components[DATETIME_MONTH] = convert_to_bcd(month);

    if (day_of_month < 1 || day_of_month > 31) {
        return false;
    }
    datetime_components[DATETIME_DAY_OF_MONTH] = convert_to_bcd(day_of_month);

    if (day_of_week > 6) {
        return false;
    }
    datetime_components[DATETIME_DAY_OF_WEEK] = convert_to_bcd(day_of_week);

    // Uses 24-hour notation by default
    if (hour > 23) {
        return false;
    }
    datetime_components[DATETIME_HOUR] = convert_to_bcd(hour);

    if (minute > 59) {
        return false;
    }
    datetime_components[DATETIME_MINUTE] = convert_to_bcd(minute);

    if (second > 59) {
        return false;
    }
    datetime_components[DATETIME_SECOND] = convert_to_bcd(second);

    return true;
}

static bool set_unix_timestamp(uint32_t seconds_since_epoch, bool sync_calendar, uint8_t timezone_diff_hours) 
{
    uint8_t ts[4] = {
        (uint8_t)seconds_since_epoch,
        (uint8_t)(seconds_since_epoch >> 8),
        (uint8_t)(seconds_since_epoch >> 16),
        (uint8_t)(seconds_since_epoch >> 24)
    };
    if (write_bytes_to_register(RV3028C7_REG_UNIX_TIME_0, ts, 4) == false) {
        return false;
    }

    if (sync_calendar) {
        // カレンダーを引数のUNIX時間と同期させる
        // ただし、タイムゾーン差分を考慮
        time_t t = seconds_since_epoch + timezone_diff_hours * 3600;
        struct tm *dt = gmtime(&t);
        if (set_datetime_components(m_datetime, dt->tm_year + 1900, dt->tm_mon + 1, dt->tm_mday, 0, dt->tm_hour, dt->tm_min, dt->tm_sec) == false) {
            return false;
        }
        if (write_bytes_to_register(RV3028C7_REG_CLOCK_SECONDS, m_datetime, DATETIME_COMPONENTS_SIZE) == false) {
            return false;
        }
    }
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

//
// 現在時刻を取得
//
static uint8_t convert_to_decimal(uint8_t bcd)
{
    return (bcd / 16 * 10) + (bcd % 16);
}

bool rv3028c7_get_timestamp(char *buf, size_t size)
{
    // レジスター（Clock register）から現在時刻を取得
    if (read_bytes_from_register(RV3028C7_REG_CLOCK_SECONDS, m_datetime, DATETIME_COMPONENTS_SIZE) == false) {
        return false;
    }

    // フォーマットして指定のバッファに設定
    if (buf != NULL) {
        snprintf(buf, size, "20%02d/%02d/%02d %02d:%02d:%02d",
                convert_to_decimal(m_datetime[DATETIME_YEAR]),
                convert_to_decimal(m_datetime[DATETIME_MONTH]),
                convert_to_decimal(m_datetime[DATETIME_DAY_OF_MONTH]),
                convert_to_decimal(m_datetime[DATETIME_HOUR]),
                convert_to_decimal(m_datetime[DATETIME_MINUTE]),
                convert_to_decimal(m_datetime[DATETIME_SECOND]));    
    }

    return true;
}

//
// 現在時刻を設定
//
bool rv3028c7_set_timestamp(uint32_t seconds_since_epoch, uint8_t timezone_diff_hours)
{
    // UNIX時間を使って時刻合わせ
    //  UNIX時間カウンターには、引数をそのまま設定し、
    //  カレンダーには、タイムゾーンに対応した時刻を設定
    //  例) 1609443121
    //    2020年12月31日 19:32:01 UTC
    //    2021年 1月 1日 04:32:01 JST <-- カレンダーから取得できるのはこちら
    if (set_unix_timestamp(seconds_since_epoch, true, timezone_diff_hours) == false) {
        NRF_LOG_DEBUG("Current timestamp setting failed");
        return false;
    }

    return true;
}
