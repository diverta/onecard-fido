/* 
 * File:   usbd_service_ccid.h
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#ifndef USBD_SERVICE_CCID_H
#define USBD_SERVICE_CCID_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#include "nrf_drv_usbd.h"
#include "app_usbd_class_base.h"
#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_util.h"

//
// app_usbd_ccid_internal
//   一部の関数、変数、マクロ等については、
//   nRF5 SDKの命名規約に沿った名称付けをしています。
//
APP_USBD_CLASS_FORWARD(app_usbd_ccid);

extern const app_usbd_class_methods_t app_usbd_ccid_class_methods;

#define APP_USBD_CCID_DEFAULT_INTERVAL 0x00

#define APP_USBD_CCID_CONFIG(iface_data, epin_data, epout_data) \
        ((iface_data, epin_data, epout_data))

#define APP_USBD_CCID_INST_CONFIG(user_event_handler, \
                                  data_ifc,           \
                                  data_ein,           \
                                  data_eout,          \
                                  ep_list)            \
        .inst = {                                     \
            .user_ev_handler = user_event_handler,    \
            .data_interface  = data_ifc,              \
            .data_epin       = data_ein,              \
            .data_epout      = data_eout,             \
            .p_ep_interval   = ep_list                \
        }

#define APP_USBD_CCID_GLOBAL_DEF_INTERNAL(instance_name,                     \
                                          user_ev_handler,                   \
                                          data_ifc,                          \
                                          data_ein,                          \
                                          data_eout)                         \
        static uint8_t CONCAT_2(instance_name, _ep) = {                      \
            (APP_USBD_CCID_DEFAULT_INTERVAL)};                               \
        APP_USBD_CLASS_INST_GLOBAL_DEF(                                      \
                instance_name,                                               \
                app_usbd_ccid,                                               \
                &app_usbd_ccid_class_methods,                                \
                APP_USBD_CCID_CONFIG(data_ifc, data_ein, data_eout),         \
                (APP_USBD_CCID_INST_CONFIG(user_ev_handler,                  \
                                           data_ifc,                         \
                                           data_ein,                         \
                                           data_eout,                        \
                                           &CONCAT_2(instance_name, _ep)))   \
                )

enum app_usbd_ccid_user_event_e;

typedef void (*app_usbd_ccid_user_ev_handler_t)(
    app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event);

// CCID I/Fインスタンス情報
typedef struct {
    uint8_t  data_interface;
    uint8_t  data_epout;
    uint8_t  data_epin;
    app_usbd_ccid_user_ev_handler_t user_ev_handler;
    uint8_t *p_ep_interval;
} app_usbd_ccid_inst_t;
#define APP_USBD_CCID_INSTANCE_SPECIFIC_DEC app_usbd_ccid_inst_t inst;

// CCID I/F共有情報
//   現状、使用していません
typedef struct {
    uint8_t dummy;
} app_usbd_ccid_ctx_t;
#define APP_USBD_CCID_DATA_SPECIFIC_DEC app_usbd_ccid_ctx_t ctx;

// CCID I/Fクラスインスタンス
APP_USBD_CLASS_TYPEDEF(app_usbd_ccid,               \
            APP_USBD_CCID_CONFIG(0, 0, 0),          \
            APP_USBD_CCID_INSTANCE_SPECIFIC_DEC,    \
            APP_USBD_CCID_DATA_SPECIFIC_DEC         \
);

// CCID I/Fイベント種別
typedef enum app_usbd_ccid_user_event_e {
    APP_USBD_CCID_USER_EVT_RX,
    APP_USBD_CCID_USER_EVT_RX_DONE,
    APP_USBD_CCID_USER_EVT_TX_DONE,
    APP_USBD_CCID_USER_EVT_PORT_OPEN,
    APP_USBD_CCID_USER_EVT_PORT_CLOSE,
} app_usbd_ccid_user_event_t;

// CCID I/Fクラスインスタンスの定義
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

//
// 関数群
//
void usbd_ccid_init(void);
void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size);
void usbd_service_ccid_do_process(void);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_CCID_H */
