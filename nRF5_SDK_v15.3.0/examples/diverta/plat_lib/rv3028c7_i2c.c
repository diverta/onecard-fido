/* 
 * File:   rv3028c7_i2c.c
 * Author: makmorit
 *
 * Created on 2020/12/30, 14:38
 */
#include "sdk_common.h"
#include "nrf_delay.h"

// for logging informations
#define NRF_LOG_MODULE_NAME rv3028c7_i2c
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#include "fido_twi.h"
#include "rv3028c7_i2c_def.h"
#include "rv3028c7_i2c.h"

// for struct tm
#include <time.h>

// テスト用
#define RV3028C7_I2C_TEST false

// 初期化処理の重複実行抑止フラグ
static bool init_done = false;

// データ送受信用の一時領域
static uint8_t read_buff[32];
static uint8_t write_buff[32];
static uint8_t m_datetime[DATETIME_COMPONENTS_SIZE];

static bool read_register(uint8_t reg_addr, uint8_t *reg_val)
{
    write_buff[0] = reg_addr;
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 1) == false) {
        NRF_LOG_ERROR("read_register: fido_twi_write fail (reg_addr=0x%02x) ", reg_addr);
        return false;
    }
    if (fido_twi_read(RV3028C7_ADDRESS, read_buff, 1) == false) {
        NRF_LOG_ERROR("read_register: fido_twi_read fail (reg_addr=0x%02x) ", reg_addr);
        return false;
    }
    *reg_val = read_buff[0];
    return true;
}

static bool read_bytes_from_registers(uint8_t reg_addr, uint8_t *data, uint8_t size) 
{
    write_buff[0] = reg_addr;
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 1) == false) {
        NRF_LOG_ERROR("read_bytes_from_registers: fido_twi_write fail (reg_addr=0x%02x) ", reg_addr);
        return false;
    }
    if (fido_twi_read(RV3028C7_ADDRESS, read_buff, size) == false) {
        NRF_LOG_ERROR("read_bytes_from_registers: fido_twi_read fail (reg_addr=0x%02x, data_size=%u) ", 
                reg_addr, size);
        return false;
    }
    memcpy(data, read_buff, size);
    return true;
}

static bool write_register(uint8_t reg_addr, uint8_t reg_val)
{
    write_buff[0] = reg_addr;
    write_buff[1] = reg_val;
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, 2) == false) {
        NRF_LOG_ERROR("write_register: fido_twi_write fail (reg_addr=0x%02x, reg_val=0x%02x) ", 
            write_buff[0], write_buff[1]);
        return false;
    }
    return true;
}

static bool write_bytes_to_registers(uint8_t reg_addr, uint8_t *data, uint8_t size) 
{
    write_buff[0] = reg_addr;
    memcpy(write_buff + 1, data, size);
    if (fido_twi_write(RV3028C7_ADDRESS, write_buff, size + 1) == false) {
        NRF_LOG_ERROR("write_bytes_to_registers: fido_twi_write fail (reg_addr=0x%02x, data_size=%u) ", 
            reg_addr, size);
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

static bool enable_trickle_charge(uint8_t tcr)
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

    // Set TCR Bits (Trickle Charge Resistor)
    //  Clear TCR Bits
    backup_reg_val &= RV3028C7_MASK_EEPROM_BACKUP_TCR_CLEAR;
    //  Shift values into EEPROM Backup Register
    backup_reg_val |= (tcr << RV3028C7_SHIFT_EEPROM_BACKUP_TCR);
    // Write 1 to TCE Bit
    backup_reg_val |= (1 << RV3028C7_BIT_EEPROM_BACKUP_TCE);

    // Write EEPROM Backup Register
    if (write_eeprom_backup_register(RV3028C7_REG_EEPROM_BACKUP, backup_reg_val) == false) {
        NRF_LOG_ERROR("Write EEPROM backup register fail");
        return false;
    }
    return true;
}

static uint8_t convert_to_decimal(uint8_t bcd)
{
    return (bcd / 16 * 10) + (bcd % 16);
}

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

static bool synchronize(uint8_t *datetime_components) 
{
    return write_bytes_to_registers(RV3028C7_REG_CLOCK_SECONDS, datetime_components, DATETIME_COMPONENTS_SIZE);
}

static bool set_unix_timestamp(uint32_t seconds_since_epoch, bool sync_calendar, uint8_t timezone_diff_hours) 
{
    uint8_t ts[4] = {
        (uint8_t)seconds_since_epoch,
        (uint8_t)(seconds_since_epoch >> 8),
        (uint8_t)(seconds_since_epoch >> 16),
        (uint8_t)(seconds_since_epoch >> 24)
    };
    if (write_bytes_to_registers(RV3028C7_REG_UNIX_TIME_0, ts, 4) == false) {
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
        if (synchronize(m_datetime) == false) {
            return false;
        }
    }
    return true;
}

bool rv3028c7_i2c_init(void)
{
    // 初期化処理の重複実行抑止
    if (init_done) {
        return true;
    } else {
        init_done = true;
    }

    // I2Cポートの初期化
    if (fido_twi_init() == false) {
        return false;
    }

    // 制御レジスター（Control 2 register）を参照、0x00なら正常
    uint8_t c2;
    if (read_register(RV3028C7_REG_CONTROL_2, &c2) == false) {
        return false;
    }
    if (c2 != 0x00) {
        return false;
    }

    //
    // 設定時刻の永続化のために、
    // 外部コンデンサー(100uF)へ充電を行うよう設定
    //
    // 0 = Switchover disabled
    // 1 = Direct Switching Mode
    // 2 = Standby Mode
    // 3 = Level Switching Mode
    uint8_t val = 0x03;
    if (set_backup_switchover_mode(val) == false) {
        return false;
    }
    // 0 =  3kOhm
    // 1 =  5kOhm
    // 2 =  9kOhm
    // 3 = 15kOhm
    uint8_t tcr = 0x03;
    if (enable_trickle_charge(tcr) == false) {
        return false;
    }

    NRF_LOG_DEBUG("RV-3028-C7 init success");
    return true;
}

bool rv3028c7_i2c_set_timestamp(uint32_t seconds_since_epoch, uint8_t timezone_diff_hours)
{
    // UNIX時間を使って時刻合わせ
    //  UNIX時間カウンターには、引数をそのまま設定し、
    //  カレンダーには、タイムゾーンに対応した時刻を設定
    //  例) 1609443121
    //    2020年12月31日 19:32:01 UTC
    //    2021年 1月 1日 04:32:01 JST <-- カレンダーから取得できるのはこちら
    if (set_unix_timestamp(seconds_since_epoch, true, timezone_diff_hours) == false) {
        return false;
    }
    return true;
}

bool rv3028c7_i2c_get_timestamp_string(char *timestamp_str)
{
    // レジスター（Clock register）から現在時刻を取得
    if (read_bytes_from_registers(RV3028C7_REG_CLOCK_SECONDS, m_datetime, DATETIME_COMPONENTS_SIZE) == false) {
        return false;
    }
    // 所定の形式でフォーマット
    sprintf(timestamp_str, "20%02d/%02d/%02d %02d:%02d:%02d",
            convert_to_decimal(m_datetime[6]), 
            convert_to_decimal(m_datetime[5]), 
            convert_to_decimal(m_datetime[4]), 
            convert_to_decimal(m_datetime[2]), 
            convert_to_decimal(m_datetime[1]), 
            convert_to_decimal(m_datetime[0]));
    return true;
}

#if RV3028C7_I2C_TEST
//
// 以下はテスト用
//
static char work_buf[64];

void rv3028c7_i2c_test(void)
{
    NRF_LOG_DEBUG("RV-3028-C7 test start");

    // RTCCの初期化
    if (rv3028c7_i2c_init() == false) {
        return;
    }

    // 現在時刻を取得
    memset(work_buf, 0, sizeof(work_buf));
    char *p = work_buf;
    if (rv3028c7_i2c_get_timestamp_string(p) == false) {
        return;
    }
    NRF_LOG_DEBUG("Current date time: %s JST", p);

    // UNIX時間を使って時刻合わせ
    uint32_t seconds_since_epoch = 1609443121;
    if (rv3028c7_i2c_set_timestamp(seconds_since_epoch, 9) == false) {
        return;
    }
    NRF_LOG_DEBUG("Set timestamp success (to 2021/01/01 04:32:01)");

    // 時刻合わせ後の現在時刻を取得
    p = work_buf + 32;
    if (rv3028c7_i2c_get_timestamp_string(p) == false) {
        return;
    }
    NRF_LOG_DEBUG("Current date time: %s JST", p);
    NRF_LOG_DEBUG("RV-3028-C7 test completed");
}
#endif // #ifdef RV3028C7_I2C_TEST
