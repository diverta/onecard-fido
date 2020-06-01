#ifndef APP_USBD_CCID_H__
#define APP_USBD_CCID_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_usbd.h"
#include "app_usbd_class_base.h"
#include "app_usbd.h"
#include "app_usbd_core.h"

#include "ccid.h"
#include "app_usbd_ccid_internal.h"

/**
 * @brief CCID class instance type.
 *
 * @ref APP_USBD_CLASS_TYPEDEF
 */
APP_USBD_CLASS_TYPEDEF(app_usbd_ccid,               \
            APP_USBD_CCID_CONFIG(0, 0, 0),          \
            APP_USBD_CCID_INSTANCE_SPECIFIC_DEC,    \
            APP_USBD_CCID_DATA_SPECIFIC_DEC         \
);

/**
 * @brief Events passed to user event handler.
 *
 * @note Example prototype of user event handler:
 *
 * @code
   void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst,
                             app_usbd_ccid_user_event_t   event);
 * @endcode
 */
typedef enum app_usbd_ccid_user_event_e {
    APP_USBD_CCID_USER_EVT_RX_DONE,     /**< User event RX_DONE.    */
    APP_USBD_CCID_USER_EVT_TX_DONE,     /**< User event TX_DONE.    */
    APP_USBD_CCID_USER_EVT_PORT_OPEN,   /**< User event PORT_OPEN.  */
    APP_USBD_CCID_USER_EVT_PORT_CLOSE,  /**< User event PORT_CLOSE. */
} app_usbd_ccid_user_event_t;

/**
 * @brief Global definition of app_usbd_ccid_t class instance.
 *
 * @param instance_name             Name of global instance.
 * @param user_ev_handler           User event handler (optional).
 * @param data_ifc                  Interface number of ccid DATA.
 * @param data_ein                  DATA subclass IN endpoint.
 * @param data_eout                 DATA subclass OUT endpoint.
 *
 * @note This macro is just simplified version of @ref APP_USBD_CCID_GLOBAL_DEF_INTERNAL.
 *
 */
#define APP_USBD_CCID_GLOBAL_DEF(instance_name,         \
                                 user_ev_handler,       \
                                 data_ifc,              \
                                 data_ein,              \
                                 data_eout)             \
    APP_USBD_CCID_GLOBAL_DEF_INTERNAL(instance_name,    \
                                      user_ev_handler,  \
                                      data_ifc,         \
                                      data_ein,         \
                                      data_eout)        \

/**
 * @brief Helper function to get class instance from CCID class.
 *
 * @param[in] p_ccid CCID class instance (defined by @ref APP_USBD_CCID_GLOBAL_DEF).
 *
 * @return Base class instance.
 */
static inline app_usbd_class_inst_t const *app_usbd_ccid_class_inst_get(app_usbd_ccid_t const *p_ccid)
{
    return &p_ccid->base;
}

/**
 * @brief Helper function to get ccid from base class instance.
 *
 * @param[in] p_inst Base class instance.
 *
 * @return CCID class handle.
 */
static inline app_usbd_ccid_t const *app_usbd_ccid_class_get(app_usbd_class_inst_t const *p_inst)
{
    return (app_usbd_ccid_t const *)p_inst;
}

//
// 関数群
//
uint8_t *app_usbd_ccid_ep_output_buffer(void);
size_t   app_usbd_ccid_ep_output_buffer_size(void);
void     app_usbd_ccid_ep_input_from_buffer(void *p_buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_CCID_H__ */
