/*
 * epd_spi_master.c
 *
 */

/*******************************************************************************
 * include.
 ******************************************************************************/
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_log.h"
#include "epd_spi_master.h"
#include "epd_config.h"


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define SPI_EPD_CS_PIN					(16)
#define SPI_INSTANCE					1		// 文字列として連結演算子で処理されるので括弧付けない
#define SPI_MISO_PIN					(11)
#define SPI_MOSI_PIN					(12)
#define SPI_SCK_PIN						(15)

#define VRAM_SIZE						(PIXEL_COUNT / 4)

/*******************************************************************************
 * static fields.
 ******************************************************************************/


static const nrf_drv_spi_t epd_spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

static uint8_t tx_data[5];								// command + 4 parameters
uint8_t tx_image_data[VRAM_SIZE + 1];					// command + image data
static uint8_t rx_data[2];								// command + data

/*******************************************************************************
 * function prototype.
 ******************************************************************************/
static void epd_spi_master_event_handler(nrf_drv_spi_evt_t const * p_event);


/*******************************************************************************
 * public function.
 ******************************************************************************/
/**
 * @brief Initialize EPD spi master module
 */
void epd_spi_master_init()
{
	uint32_t err_code;

    // Configure SPI master.
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
	spi_config.mode			= NRF_DRV_SPI_MODE_0;
    spi_config.sck_pin  	= SPI_SCK_PIN;
    spi_config.miso_pin 	= SPI_MISO_PIN;
    spi_config.mosi_pin		= SPI_MOSI_PIN;
    spi_config.ss_pin   	= NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.bit_order	= NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
	spi_config.frequency 	= NRF_DRV_SPI_FREQ_500K;
	
    err_code = nrf_drv_spi_init(&epd_spi, &spi_config, epd_spi_master_event_handler);
	
	nrf_gpio_cfg_output(SPI_EPD_CS_PIN);
	nrf_gpio_pin_set(SPI_EPD_CS_PIN);
	
    APP_ERROR_CHECK(err_code);
}


void epd_spi_master_transfer(const uint8_t *tx_buf, uint16_t tx_length, uint8_t *rx_buf, uint16_t rx_length)
{
	nrf_gpio_pin_clear(SPI_EPD_CS_PIN);
	spi_xfer_done = false;
	APP_ERROR_CHECK(nrf_drv_spi_transfer(&epd_spi, tx_buf, tx_length, rx_buf, rx_length));
	while (!spi_xfer_done)
	{
		__WFE();
	}	
	nrf_gpio_pin_set(SPI_EPD_CS_PIN);
}


/**
 * @brief Send and receive data.
 */
void epd_spi_master_transfer_large(const uint8_t *tx_buf, uint16_t tx_length, uint8_t *rx_buf, uint16_t rx_length)
{
	uint16_t sent_length = 0;
	uint16_t received_length = 0;
	uint16_t tx_remain;
	uint16_t tx_size = 0;
	uint16_t rx_remain;
	uint16_t rx_size = 0;
	uint8_t *p_tx_buff;
	uint8_t *p_rx_buff;


	nrf_gpio_pin_clear(SPI_EPD_CS_PIN);
	
	while (true)
	{		
		tx_remain = tx_length - sent_length;
		if (tx_remain & 0xff00) {
			tx_size = 255;
		} else {
			tx_size = tx_remain;
			//NRF_LOG_PRINTF("[%s] tx_remain = %d\n", __func__, tx_remain);
		}
		
		rx_remain = rx_length - received_length;
		if (rx_remain & 0xff00) {
			rx_size = 255;
		} else {
			rx_size = rx_remain;
		}
		
		if ((tx_size == 0) && (rx_size == 0)) {
			break;
		}
		
		if (tx_buf)	{
			p_tx_buff = (uint8_t*)&tx_buf[sent_length];
		} else {
			p_tx_buff = NULL;
		}
		
		if (rx_buf) {
			p_rx_buff = &rx_buf[received_length];
		} else {
			p_rx_buff = NULL;
		}
		
		spi_xfer_done = false;
		APP_ERROR_CHECK(nrf_drv_spi_transfer(&epd_spi, p_tx_buff, tx_size, p_rx_buff, rx_size));
	
		while (!spi_xfer_done)
		{
			__WFE();
		}
		sent_length += tx_size;
		received_length += rx_size;
	}

	nrf_gpio_pin_set(SPI_EPD_CS_PIN);
	
	//NRF_LOG_PRINTF("[%s] sent_length = %d\n", __func__, sent_length);
}


/**
 * @brief Write command with 1 parameter.
 */
void epd_spi_write_command_1param(uint8_t command, uint8_t param1)
{
	tx_data[0] = command & ~0x80;
	tx_data[1] = param1;
	epd_spi_master_transfer(tx_data, 2, rx_data, 2);
}


/**
 * @brief Write command with 2 parameters.
 */
void epd_spi_write_command_2params(uint8_t command, uint8_t param1, uint8_t param2)
{
	tx_data[0] = command & ~0x80;
	tx_data[1] = param1;
	tx_data[2] = param2;
	epd_spi_master_transfer(tx_data, 3, NULL, 0);
}


/**
 * @brief Write command with 3 parameters.
 */
void epd_spi_write_command_3params(uint8_t command, uint8_t param1, uint8_t param2, uint8_t param3)
{
	tx_data[0] = command & ~0x80;
	tx_data[1] = param1;
	tx_data[2] = param2;
	tx_data[3] = param3;
	epd_spi_master_transfer(tx_data, 4, NULL, 0);	
}


/**
 * @brief Write command with 4 parameters.
 */
void epd_spi_write_command_4params(uint8_t command, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4)
{
	tx_data[0] = command & ~0x80;
	tx_data[1] = param1;
	tx_data[2] = param2;
	tx_data[3] = param3;
	tx_data[4] = param4;
	epd_spi_master_transfer(tx_data, 5, NULL, 0);
}	


/**
 * @brief Write command and bulk data.
 */
void epd_spi_write_command_and_bulk_data(uint8_t command, uint8_t *buffer, uint16_t size)
{
	tx_image_data[0] = command & ~0x80;
	epd_spi_master_transfer_large(tx_image_data, size + 1, NULL, 0);
}


/**
 * @brief Write command byte repeat.
 */
void epd_spi_write_command_byte_repeat(uint8_t command, uint8_t data, uint16_t size)
{
	uint16_t i;
	tx_image_data[0] = command & ~0x80;
	for (i = 1; (i < size) && (i < VRAM_SIZE); i++)
	{
		tx_image_data[i] = data;
	}

	//NRF_LOG_PRINTF("[%s] transfer size = %d\n", __func__, i);
	epd_spi_master_transfer_large(tx_image_data, i, NULL, 0);
}


/**
 * @brief Read command 1 parameter.
 */
uint8_t epd_spi_read_command_1param(uint8_t command)
{
	tx_data[0] = command | 0x80;
	tx_data[1] = 0x00;
	epd_spi_master_transfer(tx_data, 2, rx_data, 2);
	return rx_data[1];
}


/*******************************************************************************
 * private function.
 ******************************************************************************/
static void epd_spi_master_event_handler(nrf_drv_spi_evt_t const * p_event)
{
	spi_xfer_done = true;
}
 
