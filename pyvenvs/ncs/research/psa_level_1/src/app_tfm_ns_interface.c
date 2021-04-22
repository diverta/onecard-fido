/* 
 * File:   app_tfm_ns_interface.c
 * Author: makmorit
 *
 * Created on 2021/04/21, 11:20
 */
#include <zephyr.h>

#include "tfm_api.h"
#include "tfm_ns_interface.h"

K_MUTEX_DEFINE(tfm_mutex);

int32_t tfm_ns_interface_dispatch(veneer_fn fn, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    // TFM request protected by NS lock
    if (k_mutex_lock(&tfm_mutex, K_FOREVER) != 0) {
        return (int32_t)TFM_ERROR_GENERIC;
    }

    int32_t result = fn(arg0, arg1, arg2, arg3);
    k_mutex_unlock(&tfm_mutex);
    return result;
}

enum tfm_status_e tfm_ns_interface_init(void)
{
    // The static K_MUTEX_DEFINE handles mutex init, so just return.
    return TFM_SUCCESS;
}
