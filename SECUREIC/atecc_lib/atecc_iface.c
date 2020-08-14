/* 
 * File:   atecc_iface.c
 * Author: makmorit
 *
 * Created on 2020/08/11, 12:22
 */
#include <stdlib.h>

#include "atecc_iface.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

// How long to wait after an initial wake failure for the POSt to complete.
// If Power-on self test (POST) is enabled, 
// the self test will run on waking from sleep or during power-on,
// which delays the wake reply.
#define POST_DELAY_MSEC 25

bool atecc_iface_init(ATECC_IFACE_CFG *cfg, ATECC_IFACE iface)
{
    bool status;

    if (cfg == NULL || iface == NULL) {
        fido_log_error("atecc_iface_init failed: BAD_PARAM");
        return false;
    }

    iface->mType = cfg->iface_type;
    iface->mIfaceCFG = cfg;

    status = atecc_iface_init_func(iface);
    if (status == false) {
        return status;
    }

    return true;
}

bool atecc_iface_init_func(ATECC_IFACE iface)
{
    bool status = false;

    // get method mapping to HAL methods for this interface
    hal_iface_init(iface);

    if (iface->init_func(iface)) {
        status = iface->postinit_func(iface);
    }

    return status;
}

bool atecc_iface_send_func(ATECC_IFACE iface, uint8_t *txdata, int txlength)
{
    return iface->send_func(iface, txdata, txlength);
}

bool atecc_iface_receive_func(ATECC_IFACE iface, uint8_t *rxdata, uint16_t *rxlength)
{
    return iface->receive_func(iface, rxdata, rxlength);
}

bool atecc_iface_wake_func(ATECC_IFACE iface, bool *wake_failed)
{
    bool status = iface->wake_func(iface, wake_failed);

    if (*wake_failed) {
        // The device might be performing a POST. 
        // Wait for it to complete and try again.
        atecc_delay_ms(POST_DELAY_MSEC);
        status = iface->wake_func(iface, wake_failed);
    }

    return status;
}

bool atecc_iface_idle_func(ATECC_IFACE iface)
{
    bool status;

    status = iface->idle_func(iface);
    atecc_delay_ms(1);
    return status;
}

bool atecc_iface_sleep_func(ATECC_IFACE iface)
{
    bool status;

    status = iface->sleep_func(iface);
    atecc_delay_ms(1);
    return status;
}

bool atecc_iface_release(ATECC_IFACE iface)
{
    if (iface == NULL) {
        fido_log_error("atecc_iface_release failed: BAD_PARAM");
        return false;
    }

    bool ret = hal_iface_release(iface->hal_data);
    iface->hal_data = NULL;
    return ret;
}
