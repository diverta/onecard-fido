#ifndef APP_USBD_CCID_INTERNAL_H__
#define APP_USBD_CCID_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "app_util.h"

/**
 * @defgroup app_usbd_ccid_internal USB CCID internals
 * @ingroup app_usbd_ccid
 * @brief @tagAPI52840 Internals of the USB ACM class implementation.
 * @{
 */

/**
 * @brief Forward declaration of type defined by @ref APP_USBD_CLASS_TYPEDEF in ccid class.
 *
 */
APP_USBD_CLASS_FORWARD(app_usbd_ccid);

/*lint -save -e165*/
/**
 * @brief Forward declaration of @ref app_usbd_ccid_user_event_e.
 *
 */
enum app_usbd_ccid_user_event_e;

/*lint -restore*/

/**
 * @brief User event handler.
 *
 * @param[in] p_inst    Class instance.
 * @param[in] event     User event.
 *
 */
typedef void (*app_usbd_ccid_user_ev_handler_t)(app_usbd_class_inst_t const *p_inst,
                                                enum app_usbd_ccid_user_event_e event);

/**
 * @brief CCID class part of class instance data.
 */
typedef struct {
    uint8_t data_interface;     //!< Interface number of ccid DATA.
    uint8_t data_epout;         //!< DATA subclass OUT endpoint.
    uint8_t data_epin;          //!< DATA subclass IN endpoint.

    app_usbd_ccid_user_ev_handler_t user_ev_handler; //!< User event handler.

    uint8_t * p_ep_interval;    //!< Endpoint intervals.
} app_usbd_ccid_inst_t;

/**
 * @brief Default interval value for endpoint IN
 *
 */
#define APP_USBD_CCID_DEFAULT_INTERVAL 0x00

/**
 * @brief CCID class configuration macro.
 *
 * Used by @ref APP_USBD_CCID_GLOBAL_DEF
 *
 * @param iface_data  Interface number of ccid DATA.
 * @param epin_data   DATA subclass IN endpoint.
 * @param epout_data  DATA subclass OUT endpoint.
 *
 */
#define APP_USBD_CCID_CONFIG(iface_data, epin_data, epout_data) \
        ((iface_data, epin_data, epout_data))

/**
 * @brief Specific class constant data for ccid class.
 *
 * @ref app_usbd_ccid_inst_t
 */
#define APP_USBD_CCID_INSTANCE_SPECIFIC_DEC app_usbd_ccid_inst_t inst;

/**
 * @brief Configures ccid class instance.
 *
 * @param user_event_handler    User event handler.
 * @param data_ifc              Interface number of ccid DATA.
 * @param data_ein              DATA subclass IN endpoint.
 * @param data_eout             DATA subclass OUT endpoint.
 * @param ep_list               List of endpoints and intervals
 */
#define APP_USBD_CCID_INST_CONFIG(user_event_handler,   \
                                     data_ifc,          \
                                     data_ein,          \
                                     data_eout,         \
                                     ep_list)           \
        .inst = {                                       \
                .user_ev_handler = user_event_handler,  \
                .data_interface  = data_ifc,            \
                .data_epin       = data_ein,            \
                .data_epout      = data_eout,           \
                .p_ep_interval   = ep_list              \
        }

/**
 * @brief Public ccid class interface.
 *
 */
extern const app_usbd_class_methods_t app_usbd_ccid_class_methods;

/**
 * @brief Global definition of @ref app_usbd_ccid_t class.
 *
 * @param instance_name         Name of global instance.
 * @param user_ev_handler       User event handler.
 * @param data_ifc              Interface number of ccid DATA.
 * @param data_ein              DATA subclass IN endpoint.
 * @param data_eout             DATA subclass OUT endpoint.
 */
/*lint -save -emacro(26 64 123 505 572 651, APP_USBD_CCID_GLOBAL_DEF_INTERNAL)*/
#define APP_USBD_CCID_GLOBAL_DEF_INTERNAL(instance_name,                        \
                                             user_ev_handler,                   \
                                             data_ifc,                          \
                                             data_ein,                          \
                                             data_eout)                         \
        static uint8_t CONCAT_2(instance_name, _ep) = {                         \
            (APP_USBD_CCID_DEFAULT_INTERVAL)};                                  \
        APP_USBD_CLASS_INST_GLOBAL_DEF(                                         \
                instance_name,                                                  \
                app_usbd_ccid,                                                  \
                &app_usbd_ccid_class_methods,                                   \
                APP_USBD_CCID_CONFIG(data_ifc, data_ein, data_eout),            \
                (APP_USBD_CCID_INST_CONFIG(user_ev_handler,                     \
                                              data_ifc,                         \
                                              data_ein,                         \
                                              data_eout,                        \
                                              &CONCAT_2(instance_name, _ep)))   \
                )
/*lint -restore*/

/**
 * @brief CCID class context.
 */
typedef struct {
    uint8_t dummy;
} app_usbd_ccid_ctx_t;

/**
 * @brief Specific class data for ccid class.
 *
 * @ref app_usbd_ccid_ctx_t
 */
#define APP_USBD_CCID_DATA_SPECIFIC_DEC app_usbd_ccid_ctx_t ctx;

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* APP_USBD_CCID_INTERNAL_H__ */
