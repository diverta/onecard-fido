/* 
 * File:   app_tiny_tft.c
 * Author: makmorit
 *
 * Created on 2022/06/08, 13:58
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/spi.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
LOG_MODULE_REGISTER(app_tiny_tft);

static const struct device *spi_dev;

static const struct spi_config spi_cfg = {
    .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8),
    .frequency = 4000000,
    .slave = 0,
};

//
// TFTの初期化
//
bool app_tiny_tft_initialize(void)
{
    return true;
}

//
// デバイスの初期化
//
#ifdef CONFIG_USE_TINY_TFT
static int app_tiny_tft_init(const struct device *dev)
{
    // SPI（spi4）デバイス初期化
    (void)dev;
    spi_dev = DEVICE_DT_GET(DT_NODELABEL(spi4));
    if (device_is_ready(spi_dev) == false) {
        LOG_ERR("SPI master #4 is not ready");
        return -ENOTSUP;
    }

    LOG_INF("SPI master #4 is ready");
    return 0;
}

SYS_INIT(app_tiny_tft_init, APPLICATION, CONFIG_KERNEL_INIT_PRIORITY_DEVICE);
#endif
