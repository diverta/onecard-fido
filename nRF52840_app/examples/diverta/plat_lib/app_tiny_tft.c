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
// Flag used to indicate that SPI instance completed the transfer.
static volatile bool spi_xfer_done;
static uint8_t m_rx_buf[16];
static const uint8_t m_length = sizeof(m_rx_buf);

static void spi_event_handler(nrf_drv_spi_evt_t const *p_event, void *p_context)
{
    (void)p_event;
    (void)p_context;
    spi_xfer_done = true;

    if (m_rx_buf[0] != 0) {
        NRF_LOG_DEBUG(" Received:");
        NRF_LOG_HEXDUMP_DEBUG(m_rx_buf, strlen((const char *)m_rx_buf));
    }
}

bool app_tiny_tft_initialize(uint32_t frequency)
{
    static bool init = true;
    if (init == false) {
        return true;
    }

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    if (frequency == 4000000) {
        spi_config.frequency = NRF_DRV_SPI_FREQ_4M;
    }

    uint32_t err_code = nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_drv_spi_init returns %d ", err_code);
        return false;
    }
    NRF_LOG_DEBUG("nrf_drv_spi_init returns %d ", err_code);

    // Initialize GPIOs
    fido_board_gpio_cfg_output(TFT_C_S);
    fido_board_gpio_cfg_output(TFT_RESET);
    fido_board_gpio_cfg_output(TFT_D_C);
    fido_board_gpio_cfg_output(TFT_LED);

    // Set GPIOs HIGH
    fido_board_gpio_pin_set(TFT_C_S);
    fido_board_gpio_pin_set(TFT_RESET);
    fido_board_gpio_pin_set(TFT_D_C);
    fido_board_gpio_pin_set(TFT_LED);

    // TFT initialization finished
    init = false;
    return true;
}

bool app_tiny_tft_write(uint8_t *buf, size_t len)
{
    memset(m_rx_buf, 0, m_length);
    spi_xfer_done = false;

    uint32_t err_code = nrf_drv_spi_transfer(&spi, buf, len, m_rx_buf, m_length);
    if (err_code != NRF_SUCCESS) {
        NRF_LOG_ERROR("nrf_drv_spi_transfer returns %d ", err_code);
        return false;
    }

    while (!spi_xfer_done) {
        __WFE();
    }

    return true;
}

void app_tiny_tft_set_c_s(int value)
{
    if (value > 0) {
        fido_board_gpio_pin_set(TFT_C_S);
    } else {
        fido_board_gpio_pin_clear(TFT_C_S);
    }
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
