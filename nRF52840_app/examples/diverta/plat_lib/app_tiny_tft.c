/* 
 * File:   app_tiny_tft.c
 * Author: makmorit
 *
 * Created on 2023/03/24, 11:58
 */
#include "nrf_drv_spi.h"
#include "app_util_platform.h"

// for logging informations
#define NRF_LOG_MODULE_NAME app_tiny_tft
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for GPIO use
#include "fido_board_define.h"

//
// SPI初期化
//
// SPI instance index
#define SPI_INSTANCE  1
// SPI instance
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

bool app_tiny_tft_initialize(uint32_t frequency)
{
    static bool init = true;
    if (init == false) {
        return true;
    }

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    if (frequency == 4000000) {
        spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
    }

    uint32_t err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_drv_spi_init returns %d ", err_code);
        return false;
    }
    NRF_LOG_DEBUG("nrf_drv_spi_init returns %d ", err_code);

    // Initialize GPIOs
    fido_board_gpio_cfg_output(TFT_RESET);
    fido_board_gpio_cfg_output(TFT_D_C);
    fido_board_gpio_cfg_output(TFT_LED);

    // Set GPIOs HIGH
    fido_board_gpio_pin_set(TFT_RESET);
    fido_board_gpio_pin_set(TFT_D_C);
    fido_board_gpio_pin_set(TFT_LED);

    // TFT initialization finished
    init = false;
    return true;
}

bool app_tiny_tft_write(uint8_t *buf, size_t len)
{
    uint32_t err_code = nrf_drv_spi_transfer(&spi, buf, len, NULL, 0);
    if (err_code == NRF_ERROR_BUSY) {
        NRF_LOG_ERROR("SPI driver is not ready for a new transfer");
        return false;

    } else if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_drv_spi_transfer returns %d ", err_code);
        return false;
    }
    return true;
}

void app_tiny_tft_set_c_s(int value)
{
    (void)value;
}

void app_tiny_tft_set_rst(int value)
{
    if (value > 0) {
        fido_board_gpio_pin_set(TFT_RESET);
    } else {
        fido_board_gpio_pin_clear(TFT_RESET);
    }
}

void app_tiny_tft_set_d_c(int value)
{
    if (value > 0) {
        fido_board_gpio_pin_set(TFT_D_C);
    } else {
        fido_board_gpio_pin_clear(TFT_D_C);
    }
}

void app_tiny_tft_set_led(int value)
{
    if (value > 0) {
        fido_board_gpio_pin_set(TFT_LED);
    } else {
        fido_board_gpio_pin_clear(TFT_LED);
    }
}

void app_tiny_tft_delay_ms(uint32_t ms)
{
    fido_board_delay_ms(ms);
}
