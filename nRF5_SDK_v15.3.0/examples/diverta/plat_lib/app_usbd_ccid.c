#include "sdk_common.h"

#include "app_usbd_ccid.h"
#include <inttypes.h>

/**
 * @defgroup app_usbd_ccid_internal CCID internals
 * @{
 * @ingroup app_usbd_ccid
 * @internal
 */

// for logging informations
#define NRF_LOG_MODULE_NAME ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

#define APP_USBD_CCID_DATA_IFACE_IDX 1    /**< CCID class data interface index. */
#define APP_USBD_CCID_DATA_EPIN_IDX  0    /**< CCID data class endpoint IN index. */
#define APP_USBD_CCID_DATA_EPOUT_IDX 1    /**< CCID data class endpoint OUT index. */

/**
 * @brief Auxiliary function to access ccid class instance data.
 *
 * @param[in] p_inst Class instance data.
 *
 * @return CCID class instance.
 */
static inline app_usbd_ccid_t const *ccid_get(app_usbd_class_inst_t const * p_inst)
{
    ASSERT(p_inst != NULL);
    return (app_usbd_ccid_t const *)p_inst;
}

/**
 * @brief Auxiliary function to access ccid class context data.
 *
 * @param[in] p_ccid    CCID class instance data.
 *
 * @return CCID class instance context.
 */
static inline app_usbd_ccid_ctx_t *ccid_ctx_get(app_usbd_ccid_t const * p_ccid)
{
    ASSERT(p_ccid != NULL);
    ASSERT(p_ccid->specific.p_data != NULL);
    return &p_ccid->specific.p_data->ctx;
}

/**
 * @brief User event handler.
 *
 * @param[in] p_inst        Class instance.
 * @param[in] event user    Event type.
 */
static inline void user_event_handler(app_usbd_class_inst_t const *p_inst,
                                      app_usbd_ccid_user_event_t event)
{
    app_usbd_ccid_t const *p_ccid = ccid_get(p_inst);
    if (p_ccid->specific.inst.user_ev_handler != NULL) {
        p_ccid->specific.inst.user_ev_handler(p_inst, event);
    }
}

/**
 * @brief Reset port to default state.
 *
 * @param p_inst Generic class instance.
 */
static void ccid_reset_port(app_usbd_class_inst_t const *p_inst)
{
    app_usbd_ccid_t const *p_ccid = ccid_get(p_inst);
    app_usbd_ccid_ctx_t   *p_ccid_ctx = ccid_ctx_get(p_ccid);

    // Set transfers configuration to default state.
    p_ccid_ctx->dummy = 0;
}

/**
 * @brief Control endpoint handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 */
static ret_code_t setup_event_handler(app_usbd_class_inst_t const *p_inst,
                                      app_usbd_setup_evt_t const *p_setup_ev)
{
    ASSERT(p_inst != NULL);
    ASSERT(p_setup_ev != NULL);

    ret_code_t ret = NRF_SUCCESS;
    if (app_usbd_setup_req_dir(p_setup_ev->setup.bmRequestType) == APP_USBD_SETUP_REQDIR_IN) {
        switch (app_usbd_setup_req_typ(p_setup_ev->setup.bmRequestType)) {
            case APP_USBD_SETUP_REQTYPE_STD:
                NRF_LOG_DEBUG("setup_req_std_in");
                break;
            case APP_USBD_SETUP_REQTYPE_CLASS:
                NRF_LOG_DEBUG("setup_req_class_in");
                break;
            default:
                ret = NRF_ERROR_NOT_SUPPORTED;
                break;
        }

    } else {
        // APP_USBD_SETUP_REQDIR_OUT
        switch (app_usbd_setup_req_typ(p_setup_ev->setup.bmRequestType)) {
            case APP_USBD_SETUP_REQTYPE_STD:
                NRF_LOG_DEBUG("setup_req_std_out");
                break;
            case APP_USBD_SETUP_REQTYPE_CLASS:
                NRF_LOG_DEBUG("setup_req_class_out");
                break;
            default:
                ret = NRF_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return ret;
}

/**
 * @brief Class specific endpoint transfer handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 */
static ret_code_t ccid_endpoint_ev(app_usbd_class_inst_t const *p_inst,
                                   app_usbd_complex_evt_t const *p_event)
{
    NRF_LOG_DEBUG("ccid_endpoint_ev called");

    ret_code_t ret = NRF_SUCCESS;
    if (NRF_USBD_EPIN_CHECK(p_event->drv_evt.data.eptransfer.ep)) {
        switch (p_event->drv_evt.data.eptransfer.status) {
            case NRF_USBD_EP_OK:
                NRF_LOG_DEBUG("EPIN_DATA: %02x done", p_event->drv_evt.data.eptransfer.ep);
                user_event_handler(p_inst, APP_USBD_CCID_USER_EVT_TX_DONE);
                break;
            case NRF_USBD_EP_ABORTED:
                break;
            default:
                ret = NRF_ERROR_INTERNAL;
                break;
        }

    } else if (NRF_USBD_EPOUT_CHECK(p_event->drv_evt.data.eptransfer.ep)) {
        switch (p_event->drv_evt.data.eptransfer.status) {
            case NRF_USBD_EP_OK:
                NRF_LOG_DEBUG("EPOUT_DATA: %02x done", p_event->drv_evt.data.eptransfer.ep);
                user_event_handler(p_inst, APP_USBD_CCID_USER_EVT_RX_DONE);
                break;
            case NRF_USBD_EP_WAITING:
            case NRF_USBD_EP_ABORTED:
                break;
            default:
                ret = NRF_ERROR_INTERNAL;
                break;
        }

    } else {
        ret = NRF_ERROR_NOT_SUPPORTED;
    }

    return ret;
}

/**
 * @brief @ref app_usbd_class_methods_t::event_handler
 */
static ret_code_t ccid_event_handler(app_usbd_class_inst_t const *p_inst,
                                     app_usbd_complex_evt_t const *p_event)
{
    ASSERT(p_inst != NULL);
    ASSERT(p_event != NULL);

    ret_code_t ret = NRF_SUCCESS;
    switch (p_event->app_evt.type) {
        case APP_USBD_EVT_DRV_SOF:
            break;
        case APP_USBD_EVT_DRV_RESET:
            ccid_reset_port(p_inst);
            break;
        case APP_USBD_EVT_DRV_SETUP:
            ret = setup_event_handler(p_inst, (app_usbd_setup_evt_t const *)p_event);
            break;
        case APP_USBD_EVT_DRV_EPTRANSFER:
            ret = ccid_endpoint_ev(p_inst, p_event);
            break;
        case APP_USBD_EVT_DRV_SUSPEND:
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_INST_APPEND:
            break;
        case APP_USBD_EVT_INST_REMOVE:
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            ccid_reset_port(p_inst);
            break;
        default:
            ret = NRF_ERROR_NOT_SUPPORTED;
            break;
    }

    return ret;
}

static bool ccid_feed_descriptors(app_usbd_class_descriptor_ctx_t *p_ctx,
                                  app_usbd_class_inst_t const     *p_inst,
                                  uint8_t                         *p_buff,
                                  size_t                           max_size)
{
    static uint8_t ifaces = 0;
    ifaces = app_usbd_class_iface_count_get(p_inst);
    app_usbd_ccid_t const *p_ccid = ccid_get(p_inst);

    APP_USBD_CLASS_DESCRIPTOR_BEGIN(p_ctx, p_buff, max_size);

    static uint8_t i = 0;
    for (i = 0; i < ifaces; i++) {
        // INTERFACE DESCRIPTOR
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x09);  // bLength
        APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_DESCRIPTOR_INTERFACE); // bDescriptorType = Interface

        static app_usbd_class_iface_conf_t const * p_cur_iface = NULL;
        p_cur_iface = app_usbd_class_iface_get(p_inst, i);

        uint8_t bMaxSlotIndex = CCID_NUMBER_OF_SLOTS - 1;
        uint8_t bMaxCCIDBusySlots = CCID_NUMBER_OF_SLOTS;

        APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_iface_number_get(p_cur_iface)); // bInterfaceNumber
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // bAlternateSetting
        APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_iface_ep_count_get(p_cur_iface)); // bNumEndpoints

        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x0B);  // bInterfaceClass: Chip/SmartCard
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // bInterfaceSubClass: 0=no boot
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // nInterfaceProtocol: 0=none
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // iInterface: Index of string descriptor

        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x36);  // bLength: CCID Descriptor size
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x21);  // bDescriptorType: Functional Descriptor type.
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x10);  // bcdCCID(LSB): CCID Class Spec release number (1.10)
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x01);  // bcdCCID(MSB)
        APP_USBD_CLASS_DESCRIPTOR_WRITE(bMaxSlotIndex); // bMaxSlotIndex: highest available slot on this device
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x07);  // bVoltageSupport: 5.0V/3.3V/1.8V
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x02);  // dwProtocols: Protocol T=1
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xA0);  // dwDefaultClock: 4MHz
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x0F);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xA0);  // dwMaximumClock: 4MHz
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x0F);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // bNumClockSupported : no setting from PC
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwDataRate: Default ICC I/O data rate
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xB0);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x04);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwMaxDataRate: Maximum supported ICC I/O data
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xB0);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x04);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // bNumDataRatesSupported : no setting from PC
        APP_USBD_CLASS_DESCRIPTOR_WRITE(LO(ABDATA_SIZE)); // dwMaxIFSD, B3
        APP_USBD_CLASS_DESCRIPTOR_WRITE(HI(ABDATA_SIZE)); // dwMaxIFSD, B2
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwMaxIFSD, B1B0
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwSynchProtocols
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwMechanical: no special characteristics
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xFE);  // dwFeatures
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x04);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(LO(ABDATA_SIZE + CCID_CMD_HEADER_SIZE)); // dwMaxCCIDMessageLength, B3
        APP_USBD_CLASS_DESCRIPTOR_WRITE(HI(ABDATA_SIZE + CCID_CMD_HEADER_SIZE)); // dwMaxCCIDMessageLength, B2
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // dwMaxCCIDMessageLength, B1B0
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xFF);  // bClassGetResponse
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0xFF);  // bClassEnvelope
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // wLcdLayout: 0000h no LCD
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);
        APP_USBD_CLASS_DESCRIPTOR_WRITE(0x00);  // bPINSupport: no PIN
        APP_USBD_CLASS_DESCRIPTOR_WRITE(bMaxCCIDBusySlots); // bMaxCCIDBusySlots

        // ENDPOINT DESCRIPTORS
        static uint8_t endpoints = 0;
        endpoints = app_usbd_class_iface_ep_count_get(p_cur_iface);

        static uint8_t j = 0;
        for (j = 0; j < endpoints; j++) {
            APP_USBD_CLASS_DESCRIPTOR_WRITE(0x07);                          // bLength
            APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_DESCRIPTOR_ENDPOINT);  // bDescriptorType = Endpoint

            static app_usbd_class_ep_conf_t const * p_cur_ep = NULL;
            p_cur_ep = app_usbd_class_iface_ep_get(p_cur_iface, j);
            APP_USBD_CLASS_DESCRIPTOR_WRITE(app_usbd_class_ep_address_get(p_cur_ep)); // bEndpointAddress
            APP_USBD_CLASS_DESCRIPTOR_WRITE(APP_USBD_DESCRIPTOR_EP_ATTR_TYPE_BULK);   // bmAttributes
            APP_USBD_CLASS_DESCRIPTOR_WRITE(LSB_16(NRF_DRV_USBD_EPSIZE));   // wMaxPacketSize LSB
            APP_USBD_CLASS_DESCRIPTOR_WRITE(MSB_16(NRF_DRV_USBD_EPSIZE));   // wMaxPacketSize MSB

            uint8_t ep_interval = p_ccid->specific.inst.p_ep_interval[0];
            APP_USBD_CLASS_DESCRIPTOR_WRITE(ep_interval);                   // bInterval
        }
    }

    APP_USBD_CLASS_DESCRIPTOR_END();
}

/**
 * @brief Public ccid class interface.
 *
 */
const app_usbd_class_methods_t app_usbd_ccid_class_methods = {
        .event_handler = ccid_event_handler,
        .feed_descriptors = ccid_feed_descriptors,
};

/** @} */
