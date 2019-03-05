#include "sdk_common.h"

#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "usbd_cdc_service.h"
#include "usbd_cdc_logger_interval_timer.h"

/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module. This creates and starts application timers.
 */
static void timers_init(void)
{
    ret_code_t err_code;

    // Initialize timer module.
    err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for initializing the nrf log module.
 */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}

static void idle_state_handle(void)
{
#ifdef BOARD_PCA10056
    // nRF52840 DKで開発時のみ、ログが出力されるようにする
    UNUSED_RETURN_VALUE(NRF_LOG_PROCESS());
#endif

    // Sleep CPU only if there was no interrupt 
    // since last loop processing
    __WFE();
}

int main(void)
{
    log_init();
    usbd_init();
    timers_init();

    // USB CDCを初期化
    usbd_cdc_init();
    NRF_LOG_INFO("BLE peripheral logger application started.");

    // ログ出力用タイマーをスタート
    usbd_cdc_logger_interval_timer_start();
    
    while (true) {
        usbd_cdc_do_process();
        idle_state_handle();
    }
}
