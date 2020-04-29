/* 
 * File:   usbd_service_ccid.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_error.h"

#include "usbd_service.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

//
// CCID関連
//
#include "app_usbd_ccid.h"

static void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event)
{
    switch (event) {
        default:
            break;
    }
}

APP_USBD_CCID_GLOBAL_DEF(m_app_inst_ccid, ccid_user_ev_handler, CCID_DATA_INTERFACE, CCID_DATA_EPIN, CCID_DATA_EPOUT);

void usbd_ccid_init(void)
{
    app_usbd_class_inst_t const * class_ccid = app_usbd_ccid_class_inst_get(&m_app_inst_ccid);
    ret_code_t ret = app_usbd_class_append(class_ccid);
    if (ret != NRF_SUCCESS) {
        NRF_LOG_ERROR("app_usbd_class_append(class_ccid) returns 0x%02x ", ret);
    }
    APP_ERROR_CHECK(ret);

    NRF_LOG_DEBUG("usbd_ccid_init() done");
}
