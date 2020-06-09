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

// for debug hid report
#define NRF_LOG_HEXDUMP_DEBUG_RX false
#define NRF_LOG_HEXDUMP_DEBUG_TX false

// for endpoint address
#include "usbd_service.h"

// データ送受信用
static nrf_drv_usbd_transfer_t _transfer;
static uint8_t _rx_buf[64];
static size_t  _rx_buf_size;
static uint8_t _tx_buf[64];

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

static const uint8_t USBD_FS_BOSDesc[] = {
    0x05,              /*bLength */
    0x0f,              /*bDescriptorType*/
    0x39, 0x00,        /*total length*/
    0x02,              /*Number of device capabilities*/

    /*WebUSB platform capability descriptor*/
    0x18,                   /*bLength*/
    0x10,                   /*Device Capability descriptor*/
    0x05,                   /*Platform Capability descriptor*/
    0x00,                   /*Reserved*/
    0x38, 0xB6, 0x08, 0x34, /*WebUSB GUID*/
    0xA9, 0x09, 0xA0, 0x47,
    0x8B, 0xFD, 0xA0, 0x76,
    0x88, 0x15, 0xB6, 0x65,
    0x00, 0x01,             /*Version 1.0*/
    0x01,                   /*Vendor request code*/
    0x00,                   /*No landing page*/

    /*Microsoft OS 2.0 Platform Capability Descriptor (MS_VendorCode == 0x02)*/
    0x1C,                   /*bLength*/
    0x10,                   /*Device Capability descriptor*/
    0x05,                   /*Platform Capability descriptor*/
    0x00,                   /*Reserved*/
    0xDF, 0x60, 0xDD, 0xD8, /*MS OS 2.0 GUID*/
    0x89, 0x45, 0xC7, 0x4C,
    0x9C, 0xD2, 0x65, 0x9D,
    0x9E, 0x64, 0x8A, 0x9F,
    0x00, 0x00, 0x03, 0x06, /*Windows version*/
    0xB2, 0x00,             /*Descriptor set length*/
    0x02,                   /*Vendor request code*/
    0x00                    /*Alternate enumeration code*/
};

/**
 * @brief Internal SETUP standard IN request handler.
 *
 * @param[in] p_inst        Generic class instance.
 * @param[in] p_setup_ev    Setup event.
 *
 * @return Standard error code.
 */
static ret_code_t setup_req_std_in(app_usbd_class_inst_t const *p_inst,
                                   app_usbd_setup_evt_t const *p_setup_ev)
{
    app_usbd_setup_reqrec_t req_rec = app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType);
    NRF_LOG_DEBUG("bmRequestType=%u, bRequest=%u", req_rec, p_setup_ev->setup.bRequest);
    
    // Only Get Descriptor standard IN request is supported by CCID class
    if ((req_rec == APP_USBD_SETUP_REQREC_DEVICE) &&
        (p_setup_ev->setup.bRequest == APP_USBD_SETUP_STDREQ_GET_DESCRIPTOR)) {
        size_t dsc_len = 0;
        size_t max_size;
        uint8_t *p_trans_buff = app_usbd_core_setup_transfer_buff_get(&max_size);

        NRF_LOG_DEBUG("Request for get descriptor: type=%02x, index=%02x", 
            p_setup_ev->setup.wValue.hb, p_setup_ev->setup.wValue.lb);

        if (p_setup_ev->setup.wValue.hb == 0x0f) {
            dsc_len = sizeof(USBD_FS_BOSDesc);
            return app_usbd_core_setup_rsp(&(p_setup_ev->setup), USBD_FS_BOSDesc, dsc_len);
            
        } else {
            // Try to find descriptor in class internals
            ret_code_t ret = app_usbd_class_descriptor_find(
                p_inst,
                p_setup_ev->setup.wValue.hb,
                p_setup_ev->setup.wValue.lb,
                p_trans_buff,
                &dsc_len);

            if (ret != NRF_ERROR_NOT_FOUND) {
                ASSERT(dsc_len < NRF_DRV_USBD_EPSIZE);
                NRF_LOG_DEBUG("Found descriptor in class internals");
                return app_usbd_core_setup_rsp(&(p_setup_ev->setup), p_trans_buff, dsc_len);

            } else {
                NRF_LOG_ERROR("Unable to find descriptor in class internals");
            }
        }  
    }

    return NRF_ERROR_NOT_SUPPORTED;
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
                setup_req_std_in(p_inst, p_setup_ev);
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

static void prepare_ep_output_buffer(app_usbd_class_inst_t const *p_inst, app_usbd_complex_evt_t const *p_event) 
{
    nrf_drv_usbd_ep_t ep_addr = p_event->drv_evt.data.eptransfer.ep;
    _transfer.p_data.rx = _rx_buf;
    _transfer.size = nrf_drv_usbd_epout_size_get(ep_addr);

    ret_code_t ret = nrf_drv_usbd_ep_transfer(ep_addr, &_transfer);
    _rx_buf_size = _transfer.size;

#if NRF_LOG_HEXDUMP_DEBUG_RX
    NRF_LOG_DEBUG("nrf_drv_usbd_ep_transfer(%d bytes) returns %d", _rx_buf_size, ret);
    NRF_LOG_HEXDUMP_DEBUG(_rx_buf, _rx_buf_size);
#endif
}

uint8_t *app_usbd_ccid_ep_output_buffer(void)
{
    return _rx_buf;
}

size_t app_usbd_ccid_ep_output_buffer_size(void)
{
    return _rx_buf_size;
}

void app_usbd_ccid_ep_input_from_buffer(void *p_buf, size_t size)
{
    memcpy(_tx_buf, p_buf, size);
    
    nrf_drv_usbd_ep_t ep_addr = CCID_DATA_EPIN;
    _transfer.p_data.tx = _tx_buf;
    _transfer.size = size;
    _transfer.flags = 0;

    ret_code_t ret = nrf_drv_usbd_ep_transfer(ep_addr, &_transfer);

#if NRF_LOG_HEXDUMP_DEBUG_TX
    NRF_LOG_DEBUG("nrf_drv_usbd_ep_transfer(%d bytes) returns %d", _transfer.size, ret);
    NRF_LOG_HEXDUMP_DEBUG(_transfer.p_data.tx, _transfer.size);
#endif
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
    ret_code_t ret = NRF_SUCCESS;
    if (NRF_USBD_EPIN_CHECK(p_event->drv_evt.data.eptransfer.ep)) {
        switch (p_event->drv_evt.data.eptransfer.status) {
            case NRF_USBD_EP_OK:
                user_event_handler(p_inst, APP_USBD_CCID_USER_EVT_TX_DONE);
                break;
            case NRF_USBD_EP_ABORTED:
                NRF_LOG_ERROR("NRF_USBD_EPIN: NRF_USBD_EP_ABORTED");
                break;
            default:
                NRF_LOG_DEBUG("NRF_USBD_EPIN: %u", p_event->drv_evt.data.eptransfer.status);
                ret = NRF_ERROR_INTERNAL;
                break;
        }

    } else if (NRF_USBD_EPOUT_CHECK(p_event->drv_evt.data.eptransfer.ep)) {
        switch (p_event->drv_evt.data.eptransfer.status) {
            case NRF_USBD_EP_OK:
                user_event_handler(p_inst, APP_USBD_CCID_USER_EVT_RX_DONE);
                break;
            case NRF_USBD_EP_WAITING:
                prepare_ep_output_buffer(p_inst, p_event);
                break;
            case NRF_USBD_EP_ABORTED:
                NRF_LOG_ERROR("NRF_USBD_EPOUT: NRF_USBD_EP_ABORTED");
                break;
            default:
                NRF_LOG_DEBUG("NRF_USBD_EPOUT: %u", p_event->drv_evt.data.eptransfer.status);
                ret = NRF_ERROR_INTERNAL;
                break;
        }

    } else {
        NRF_LOG_ERROR("unknown ep: %u", p_event->drv_evt.data.eptransfer.status);
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
        case APP_USBD_EVT_DRV_SUSPEND:
            break;
        case APP_USBD_EVT_DRV_RESUME:
            break;
        case APP_USBD_EVT_DRV_WUREQ:
            break;
        case APP_USBD_EVT_DRV_SETUP:
            ret = setup_event_handler(p_inst, (app_usbd_setup_evt_t const *)p_event);
            break;
        case APP_USBD_EVT_DRV_EPTRANSFER:
            ret = ccid_endpoint_ev(p_inst, p_event);
            break;
        case APP_USBD_EVT_POWER_REMOVED:
            ccid_reset_port(p_inst);
            break;
        case APP_USBD_EVT_INST_APPEND:
            break;
        case APP_USBD_EVT_INST_REMOVE:
            break;
        case APP_USBD_EVT_STARTED:
            break;
        case APP_USBD_EVT_STOPPED:
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
        APP_USBD_CLASS_DESCRIPTOR_WRITE(LO(APDU_DATA_SIZE)); // dwMaxIFSD, B3
        APP_USBD_CLASS_DESCRIPTOR_WRITE(HI(APDU_DATA_SIZE)); // dwMaxIFSD, B2
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
        APP_USBD_CLASS_DESCRIPTOR_WRITE(LO(APDU_DATA_SIZE + CCID_CMD_HEADER_SIZE)); // dwMaxCCIDMessageLength, B3
        APP_USBD_CLASS_DESCRIPTOR_WRITE(HI(APDU_DATA_SIZE + CCID_CMD_HEADER_SIZE)); // dwMaxCCIDMessageLength, B2
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
