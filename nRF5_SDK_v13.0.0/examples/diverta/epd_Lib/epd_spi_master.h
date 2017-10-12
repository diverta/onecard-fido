/*
 * epd_spi_master.h
 */

#ifndef EPD_SPI_MASTER_H_
#define EPD_SPI_MASTER_H_

/*******************************************************************************
 * include.
 ******************************************************************************/
#include <stdint.h>

/*******************************************************************************
 * constant definition.
 ******************************************************************************/
/*
 * spi master.
 */
#define SPI_TX_RX_LENGTH					(180)



/*******************************************************************************
 * static fields.
 ******************************************************************************/


/*******************************************************************************
 * function prototype.
 ******************************************************************************/
/*
 * spi master.
 */
void epd_spi_master_init(void);
void epd_spi_write_command_1param(uint8_t command, uint8_t param1);
void epd_spi_write_command_2params(uint8_t command, uint8_t param1, uint8_t param2);
void epd_spi_write_command_3params(uint8_t command, uint8_t param1, uint8_t param2, uint8_t param3);
void epd_spi_write_command_4params(uint8_t command, uint8_t param1, uint8_t param2, uint8_t param3, uint8_t param4);
void epd_spi_write_command_and_bulk_data(uint8_t command, uint8_t *buffer, uint16_t size);
void epd_spi_write_command_byte_repeat(uint8_t command, uint8_t data, uint16_t size);
uint8_t epd_spi_read_command_1param(uint8_t command);
//void epd_spi_read_command_bulk_data(uint8_t command, uint8_t *buffer, uint16_t size);

void epd_spi_master_transfer_large(const uint8_t *tx_buf, uint16_t tx_length, uint8_t *rx_buf, uint16_t rx_length);
void epd_spi_master_transfer(const uint8_t *tx_buf, uint16_t tx_length, uint8_t *rx_buf, uint16_t rx_length);


#endif	// EPD_SPI_MASTER_H_
