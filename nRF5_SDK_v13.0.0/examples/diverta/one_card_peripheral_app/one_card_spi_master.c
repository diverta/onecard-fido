/*
 * one_card_spi_master.c
 *
 *  Created on: 2016/03/23
 *      Author: r.suzuki
 */


/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdio.h>

#include "nordic_common.h"
#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_drv_spi.h"
#include "app_util_platform.h"

#include "one_card_config.h"
#include "one_card_spi_master.h"


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
#define SPI_INSTANCE					0


/*******************************************************************************
 * static fields.
 ******************************************************************************/
static const nrf_drv_spi_t				m_spi_instance = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);//!< instances of the SPI master driver.
static uint8_t							m_spi_tx_buf[SPI_TX_RX_LENGTH]; 					//!< SPI master TX buffer.
static uint8_t							m_spi_rx_buf[SPI_TX_RX_LENGTH]; 					//!< SPI master RX buffer.
static uint8_t							m_spi_transfer_length;								//!< transfer length(<=SPI_TX_RX_LENGTH).
static volatile bool					m_spi_xfer_done;									//!< Flag used to indicate that SPI instance completed the transfer.


/*******************************************************************************
 * function prototype.
 ******************************************************************************/
static void on_spi_evt(nrf_drv_spi_evt_t const *spi_master_evt, void *p_context);


/*******************************************************************************
 * public function.
 ******************************************************************************/
/**
 * @brief Function for initializing the spi master module.
 */
void one_card_spi_master_init(void)
{
    ret_code_t err_code;

    // Configure SPI master.
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.mode			= NRF_DRV_SPI_MODE_0;
    spi_config.sck_pin  	= PIN_SPI_SCK;
    spi_config.miso_pin 	= PIN_SPI_MISO;
    spi_config.mosi_pin		= PIN_SPI_MOSI;
    spi_config.ss_pin   	= NRF_DRV_SPI_PIN_NOT_USED;
    spi_config.bit_order	= NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
    spi_config.frequency 	= NRF_DRV_SPI_FREQ_125K;	// 125     kbps.
    //spi_config.frequency 	= (0x01000000UL);			//  62.5   kbps.
    //spi_config.frequency 	= (0x00800000UL);			//  31.25  kbps.
    //spi_config.frequency 	= (0x00400000UL);			//  15.625 kbps.

    // Register event handler for SPI master.
    err_code = nrf_drv_spi_init(&m_spi_instance, &spi_config, on_spi_evt, NULL);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief Function for sending and receiving data.
 */
void one_card_spi_master_transfer(void)
{
	// cleanup.
	m_spi_xfer_done = false;
	
    if (0 < m_spi_transfer_length) {
        uint32_t err_code;
        err_code = nrf_drv_spi_transfer(&m_spi_instance, m_spi_tx_buf, SPI_TX_RX_LENGTH, m_spi_rx_buf, SPI_TX_RX_LENGTH);
        APP_ERROR_CHECK(err_code);
		
		NRF_LOG_INFO("[SPI]spi transfer start.\r\n");
		
		// Wait for transfer completion.
        //while (!m_spi_xfer_done) {
        //    __WFE();
        //}
		
        //NRF_LOG_FLUSH();
    }
}

/**
 * @brief Function for set the spi tx data.
 *
 * @param[in]	p_data
 * @param[in]	length
 */
void one_card_spi_master_set_tx_data(const uint8_t *p_data, uint16_t length)
{
	// clear tx & rx buffer.
	for (int i = 0; i < SPI_TX_RX_LENGTH; i++) {
		m_spi_tx_buf[i] = 0x00;
		m_spi_rx_buf[i] = 0x00;
	}
	m_spi_transfer_length = 0;

	// available transmission size?
	if (length <= SPI_TX_RX_LENGTH) {
		// set the tx data.
		for (int i = 0; i < length; i++) {
			m_spi_tx_buf[i] = p_data[i];
		}
		m_spi_transfer_length = length;
	}
	else {
		NRF_LOG_WARNING("[SPI]It is over the available transmission size : %d\r\n", length);
	}
}


/*******************************************************************************
 * ptivate function.
 ******************************************************************************/
/**
 * @brief spi master event handler.
 *
 * @param[in]	spi_master_evt
 * @param[in]	p_context
 */
static void on_spi_evt(nrf_drv_spi_evt_t const *spi_master_evt, void *p_context)
{
	m_spi_xfer_done = true;
	
	NRF_LOG_INFO("[SPI]spi transfer completed\r\n");
	
	NRF_LOG_INFO("[SPI]spi tx : \r\n");
	NRF_LOG_HEXDUMP_INFO(m_spi_tx_buf, SPI_TX_RX_LENGTH);
	//NRF_LOG_HEXDUMP_INFO(m_spi_tx_buf, strlen((const char *)m_spi_tx_buf));
	
	NRF_LOG_INFO("[SPI]spi rx : \r\n");
	NRF_LOG_HEXDUMP_INFO(m_spi_rx_buf, SPI_TX_RX_LENGTH);
	//NRF_LOG_HEXDUMP_INFO(m_spi_rx_buf, strlen((const char *)m_spi_rx_buf));
	
	NRF_LOG_FLUSH();
}
