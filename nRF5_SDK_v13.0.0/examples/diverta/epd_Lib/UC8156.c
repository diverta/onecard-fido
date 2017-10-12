/*
 * UC8156.c
 *
 *  Created on: 2016/05/11
 *		Author: K.Shoji
 */

/*******************************************************************************
 * include.
 ******************************************************************************/
#include "UC8156.h"
#include "epd_config.h"
#include "epd_spi_master.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define EPD_BUSY_PIN			(8)
#define EPD_POWER_EN_PIN		(18)
#define EPD_RESET_PIN			(20)

/*******************************************************************************
 * static fields.
 ******************************************************************************/
static uint8_t panel_setting = 0x00;
static uint8_t driver_voltage_setting1 = 0x50;
static uint8_t driver_voltage_setting2 = 0xFF;

/*******************************************************************************
 * function prototype.
 ******************************************************************************/


/*******************************************************************************
 * public function.
 ******************************************************************************/
/**
 * @brief Config interface
 */
void UC8156_configure_interface()
{
	nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(EPD_RESET_PIN);
	nrf_gpio_cfg_output(EPD_POWER_EN_PIN);
	nrf_gpio_pin_clear(EPD_RESET_PIN);
	nrf_gpio_pin_set(EPD_POWER_EN_PIN);
	epd_spi_master_init();
}


/**
 * @brief Power ON
 */
void UC8156_power_on(void)
{
	nrf_gpio_pin_set(EPD_POWER_EN_PIN);
}


/**
 * @brief Power OFF
 */
void UC8156_power_off(void)
{
	nrf_gpio_pin_clear(EPD_POWER_EN_PIN);
}


/**
 * @brief Reset IC.
 */
void UC8156_hardware_reset()
{
	nrf_gpio_pin_clear(EPD_RESET_PIN);
	nrf_delay_ms(1);
	nrf_gpio_pin_set(EPD_RESET_PIN);
}


/**
 * @breief Wait for BUSY inactive.
 */
void UC8156_wait_for_BUSY_inactive()
{
	while (nrf_gpio_pin_read(EPD_RESET_PIN) == 0);		// BUSY loop
}


/**
 * @brief Initialize registers.
 */
void UC8156_init_registers()
{
	// REG[01h] CMD_PANEL_SETTING
	// GVS=1 (p-mos, selected gate to VGL and non-selected gate to VGH) + SOO=1
	panel_setting = 0x12;
	epd_spi_write_command_1param(CMD_PANEL_SETTING, panel_setting);
		
	// REG[02h] CMD_DRIVER_VOLTAGE_SETTING
	// set Vgate to +17V/-25V
	driver_voltage_setting1 = 0x25;		// VGH_LV = 17V, VGL_LV = -25V
	driver_voltage_setting2 = 0xFF;		// VSH_VL = 15V, VSL_LV = -15V
	epd_spi_write_command_2params(CMD_DRIVER_VOLTAGE_SETTING, driver_voltage_setting1, driver_voltage_setting2);

	// REG[05h] CMD_VCOM_AND_DATA_INTERVAL_SETTING	サンプルコードになし
	
	// REG[06h] CMD_TCOM_TIMING_SETTING
	// set timing to LAT=105, S2G+G2S=5
	epd_spi_write_command_2params(CMD_TCOM_TIMING_SETTING, 0x67, 0x55);	

	// REG[18h] CMD_VCOM_CONFIG_REGISTER
	// BPCOM=GND, TPCOM=GND after update, gate_out=GND after update
	epd_spi_write_command_4params(CMD_VCOM_CONFIG_REGISTER, 0x00, 0x00, 0x24, 0x07);
	// BPCOM=GND, TPCOM=Hi-Z after update, gate_out=GND after update
	//epd_spi_write_command_4params(CMD_VCOM_CONFIG_REGISTER, 0x40, 0x00, 0x24, 0x07);
	
	// REG[1Dh] CMD_VBORDER_SETTING
	// set Vborder to Vcom
	epd_spi_write_command_1param(CMD_VBORDER_SETTING, 0x04);
	
	// REG[1Fh] CMD_POWER_SEQUENCE_SETTING
	// set all NF's and waiting times to 0
	epd_spi_write_command_3params(CMD_POWER_SEQUENCE_SETTING, 0x00, 0x00, 0x00);

	// image RAM configuration
	
	// REG[0Fh] CMD_DATA_ENTRY_MODE_SETTING
	// DEM=010 --> Y-decrement
	epd_spi_write_command_1param(CMD_DATA_ENTRY_MODE_SETTING, 0x02);
	
	// REG[0Ch] CMD_PANEL_RESOLUTION_SETTING
	// Panel resolution setting --> SOURCE_E needs to be SOURCELINE instead of SOURCELINE-1 for 180x100, don't know why	
	epd_spi_write_command_4params(CMD_PANEL_RESOLUTION_SETTING, 0x00, SOURCE_LINES, GATE_LINES_MAX - GATE_LINES, GATE_LINES_MAX - 1);
	//epd_spi_write_command_4params(CMD_PANEL_RESOLUTION_SETTING, 0x00, SOURCE_LINES - 1, GATE_LINES_MAX - GATE_LINES, GATE_LINES_MAX - 1);

	// REG[0Dh] CMD_WHITE_PIXEL_RECT_SETTING
	// RAM window setup
	epd_spi_write_command_4params(CMD_WHITE_PIXEL_RECT_SETTING, 0x00, SOURCE_LINES - 1, GATE_LINES_MAX - GATE_LINES, GATE_LINES_MAX - 1);
	
	// REG[0Eh] CMD_PIXEL_ACCESS_POSITION_SETTING
	// start Y from 159d/9fh, related to R0fh/DEM setting	
	epd_spi_write_command_2params(CMD_PIXEL_ACCESS_POSITION_SETTING, 0x00, GATE_LINES_MAX - 1);
	
	// REG[03h] CMD_POWER_CONTROL_SETTING サンプルコードになし
	// → 後の HV power-on で操作している
}


/**
 * @breif HV power-on
 */
void UC8156_HVs_on()
{
	uint8_t reg_value = epd_spi_read_command_1param(CMD_POWER_CONTROL_SETTING);	// read power control setting register
	reg_value |= 0x11;															// switch on CLKEN + PWRON bits
	epd_spi_write_command_1param(CMD_POWER_CONTROL_SETTING, reg_value);			// write power control setting register --> switch on CLKEN+PWRON bits
	UC8156_wait_for_BUSY_inactive();
 	while (epd_spi_read_command_1param(CMD_STATUS_REGISTER) != 4);				// BUSY loop
	epd_spi_read_command_1param(0x00);
}


/**
 * @brief HV power-off
 */
void UC8156_HVs_off()
{
	uint8_t reg_value = epd_spi_read_command_1param(CMD_POWER_CONTROL_SETTING);	// read power control setting register
	reg_value &= ~0x01;															// switch off PWRON bit
	epd_spi_write_command_1param(CMD_POWER_CONTROL_SETTING, reg_value);			// write power control setting register
	UC8156_wait_for_BUSY_inactive();
	reg_value &= ~0x10;															// switch off CLKEN bit
	epd_spi_write_command_1param(CMD_POWER_CONTROL_SETTING, reg_value);	
}


/**
 * @brief Read revision ID.
 */
uint8_t UC8156_read_RevID()
{
	return epd_spi_read_command_1param(CMD_REVISION);
}


/**
 * @brief Send waveform data.
 */
void UC8156_send_waveform(uint8_t *waveform)
{
	epd_spi_write_command_and_bulk_data(CMD_WAVEFORM_LUT_SETTING, waveform, WAVEFORM_LENGTH);	
}


/**
 * @brief Set Vcom.
 */
void UC8156_set_Vcom(int Vcom_mv_value)
{
	uint16_t Vcom_register_value = (float)Vcom_mv_value/(float)30.0;
	epd_spi_write_command_2params(CMD_VCOM_DC_SETTING, (uint8_t)Vcom_register_value, (uint8_t)((Vcom_register_value>>8)&0x03));
}


/**
 * @brief Send image data.
 */
void UC8156_send_image_data(uint8_t *image_data)
{
	epd_spi_write_command_and_bulk_data(CMD_WRITE_RAM, image_data, PIXEL_COUNT / 4);
}


/**
 * @brief Send repeated image data.
 */
void UC8156_send_repeated_image_data(uint8_t image_data)
{
	epd_spi_write_command_byte_repeat(CMD_WRITE_RAM, image_data, PIXEL_COUNT / 4);	
}


/**
 * @brief Update display.
 */
void UC8156_update_display(uint8_t mode)
{
	epd_spi_write_command_1param(CMD_DISPLAY_ENGINE_CONTROL_REGISTER, UPDATE_WAVEFORMSOURCESELECT | mode | 1);
	UC8156_wait_for_BUSY_inactive();
}


/**
 * @brief Send data to image RAM for MTP program.
 */
void UC8156_send_data_to_image_RAM_for_MTP_program(uint8_t *waveform_data, uint16_t size)
{
	epd_spi_write_command_and_bulk_data(CMD_WRITE_RAM, waveform_data, size);	
}
 

/**
 * @brief Read status regiser
 */
uint8_t UC8156_read_status_register(void)
{
	return epd_spi_read_command_1param(CMD_STATUS_REGISTER);
}


 /*******************************************************************************
 * private function.
 ******************************************************************************/

