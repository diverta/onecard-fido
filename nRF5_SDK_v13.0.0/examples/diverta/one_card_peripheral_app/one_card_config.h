/*
 * one_card_config.h
 *
 *  Created on: 2017/06/15
 *      Author: r.suzuki
 */

#ifndef ONECARD_CONFIG_H_
#define ONECARD_CONFIG_H_

/*******************************************************************************
 * include.
 ******************************************************************************/


/*******************************************************************************
 * constant definition.
 ******************************************************************************/
// nRF51 PCA10028
#ifdef NRF51

#define PIN_SCAN_SW_IN						0
#define PIN_DRV_TRIG_OUT					1
#define PIN_MAIN_SW_IN						2
#define PIN_PWR_CONT_OUT					3
#define PIN_WAKEUP_OUT						5
#define PIN_EPD_BUSY_N						8
#define PIN_SPI_AT_CS						9
#define PIN_SPI_MISO						11
#define PIN_SPI_MOSI						12
#define PIN_SPI_SCK							15
#define PIN_SPI_EPD_CS						16
#define PIN_EPD_POWER_EN_OUT				18
#define PIN_EPD_RESET_OUT					20

// nRF52 PCA10040
#elif NRF52

#define PIN_SCAN_SW_IN						2
#define PIN_DRV_TRIG_OUT					3
#define PIN_MAIN_SW_IN						4
#define PIN_PWR_CONT_OUT					28
#define PIN_WAKEUP_OUT						30
#define PIN_EPD_BUSY_N						7
#define PIN_SPI_AT_CS						8
#define PIN_SPI_MISO						10
#define PIN_SPI_MOSI						11
#define PIN_SPI_SCK							14
#define PIN_SPI_EPD_CS						15
#define PIN_EPD_POWER_EN_OUT				17
#define PIN_EPD_RESET_OUT					19

#endif


#define PILOT_LED_FORCED_FLASH				1											//!< If this directive is enabled, pilot led forced flash.
#define ENABLED_PILOT_LED_FLASH				1											//!< Pilot led do flash.
#define FIXED_PULSE_TEST_MODE				1											//!< Outputs fixed pulse instaed of F2F pulse.


/*******************************************************************************
 * type definition.
 ******************************************************************************/


/*******************************************************************************
 * structure.
 ******************************************************************************/


#endif /* ONECARD_CONFIG_H_ */
