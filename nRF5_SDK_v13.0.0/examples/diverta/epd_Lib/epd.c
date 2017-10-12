/*
 * epd.c
 *
 *  Created on: 2016/05/13
 *		Author: K.Shoji
 */

/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include "UC8156.h"
#include "epd_config.h"
#include "epd.h"
#include "nrf_log.h"
#include "epd_spi_master.h"
#include "nrf_delay.h"
#include "epd_graphics.h"
#include "gfxfont.h"

/*******************************************************************************
 * constant definition.
 ******************************************************************************/
//#define BUFF_SIZE		(PIXEL_COUNT / 4)

/*******************************************************************************
 * macros
 ******************************************************************************/


/*******************************************************************************
 * static fields.
 ******************************************************************************/
static const uint8_t width = SOURCE_LINES;
static const uint8_t height = GATE_LINES;
static uint8_t *image_buff = NULL;
static uint16_t image_buff_size = 0;

/*******************************************************************************
 * function prototype.
 ******************************************************************************/


/*******************************************************************************
 * public function.
 ******************************************************************************/
/**
 * @brief Function for initializing EPD
 */
void epd_init(void)
{
	image_buff_size = get_image_buff_size();
	image_buff = get_image_buff_addr();
	
	UC8156_configure_interface();
	epd_power_on();
	
	/* ここのくだりはデータシートのパワーオンシーケンスに無いのでコメントアウトしてみる	
	UC8156_wait_for_BUSY_inactive();		// Wait for BUSY_N = high
	nrf_delay_ms(5);
	*/
	
	UC8156_hardware_reset();				// UC8156 hardware reset
	UC8156_wait_for_BUSY_inactive();		// Wait for BUSY_N = high
	UC8156_init_registers();				// Power ON Sequence
	UC8156_wait_for_BUSY_inactive();		// Wait for BUSY_N = high	
	
	uint8_t status_reg = UC8156_read_status_register();
	//NRF_LOG_PRINTF("[%s] Status Register = %x\n", __func__, status_reg);
	
	uint8_t revID = UC8156_read_RevID();
	//NRF_LOG_PRINTF("[%s] RevID = %x\n", __func__, revID);
	
	//UC8156_set_Vcom(3900);
	epd_graphics_init();
}


/**
 * @brief EPD Power ON
 */
void epd_power_on(void)
{
	UC8156_power_on();
}


/**
 * @brief EPD Power OFF
 */
void epd_power_off(void)
{
	UC8156_power_off();
}


/**
 * @brief Clear display
 */
void epd_clear_display(void)
{
	uint8_t reg0fh_value = epd_spi_read_command_1param(0x0f);
	
	fill_screen(3);
	
	epd_spi_write_command_1param(CMD_DATA_ENTRY_MODE_SETTING, reg0fh_value | DATA_ENTRY_MODE_BIT_RAMSEL);
	UC8156_send_image_data(image_buff);
	epd_spi_write_command_1param(CMD_DATA_ENTRY_MODE_SETTING, reg0fh_value & (~DATA_ENTRY_MODE_BIT_RAMSEL));
	UC8156_send_image_data(image_buff);
	
	UC8156_HVs_on();
	UC8156_update_display(FULL_UPDATE);
	UC8156_HVs_off();
}	


/**
 * @brief Send image buff to display and update.
 */
void epd_update(void)
{
	if (image_buff != NULL)
	{
		nrf_delay_ms(500);
		UC8156_send_image_data(image_buff);
		UC8156_HVs_on();
		UC8156_update_display(FULL_UPDATE);
		UC8156_HVs_off();		
	}
}


/**
 * @brief Test graphics
 */
void epd_test_graphics(void)
{
	fill_screen(3);

	draw_rect(0, 0, 20, 20, 0);
	//draw_line(171, 98, 179, 98, 0);
	//draw_line(10, 0, 20, 10, 0);
	//draw_line(0, 1, 5, 1, 0);
	//draw_pixel(1, 5, 0);
	//image_buff[0] = 0x00;
	//image_buff[1] = 0x00;
	epd_update();
}


/**
 * @brief Test writing character functions
 */
void epd_test_write_char(void)
{
	// 文字描画
	//draw_char(0, 80, 'A', 0, 1);
	
	set_cursor(0, 40);
	/*
	write('A');
	write('B');
	write('C');
	write('D');
	*/
	write_str("EPD test program... check one, two...");
	epd_update();
}












