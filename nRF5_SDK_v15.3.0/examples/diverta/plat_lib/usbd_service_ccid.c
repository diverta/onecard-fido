/* 
 * File:   usbd_service_ccid.c
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#include "app_error.h"

//
// CCID関連
//
#include "usbd_service.h"
#include "usbd_service_bos.h"
#include "usbd_service_ccid.h"
#include "ccid.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_ccid
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

// for debug buffer
#define NRF_LOG_HEXDUMP_DEBUG_BUFFER false
#define NRF_LOG_HEXDUMP_DEBUG_RX false
#define NRF_LOG_HEXDUMP_DEBUG_TX false

//
// app_usbd_ccid_internal
//   一部の関数、変数、マクロ等については、
//   nRF5 SDKの命名規約に沿った名称付けをしています。
//
// データ送受信用の一時変数
static nrf_drv_usbd_transfer_t _transfer;
static uint8_t _rx_buf[64];
static size_t  _rx_buf_size;
static uint8_t _tx_buf[64];

static inline app_usbd_ccid_t const *ccid_get(app_usbd_class_inst_t const * p_inst)
{
    ASSERT(p_inst != NULL);
    return (app_usbd_ccid_t const *)p_inst;
}

static inline app_usbd_ccid_ctx_t *ccid_ctx_get(app_usbd_ccid_t const * p_ccid)
{
    ASSERT(p_ccid != NULL);
    ASSERT(p_ccid->specific.p_data != NULL);
    return &p_ccid->specific.p_data->ctx;
}

static inline void user_event_handler(app_usbd_class_inst_t const *p_inst,
                                      app_usbd_ccid_user_event_t event)
{
    app_usbd_ccid_t const *p_ccid = ccid_get(p_inst);
    if (p_ccid->specific.inst.user_ev_handler != NULL) {
        p_ccid->specific.inst.user_ev_handler(p_inst, event);
    }
}

static void ccid_reset_port(app_usbd_class_inst_t const *p_inst)
{
    app_usbd_ccid_t const *p_ccid = ccid_get(p_inst);
    app_usbd_ccid_ctx_t   *p_ccid_ctx = ccid_ctx_get(p_ccid);

    // Set transfers configuration to default state.
    p_ccid_ctx->dummy = 0;
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
            ret = usbd_service_bos_setup(p_event);
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

const app_usbd_class_methods_t app_usbd_ccid_class_methods = {
        .event_handler = ccid_event_handler,
        .feed_descriptors = ccid_feed_descriptors,
};

//
// プラットフォーム〜業務処理間のインターフェース
//
static void ccid_user_ev_handler(app_usbd_class_inst_t const *p_inst, enum app_usbd_ccid_user_event_e event)
{
    switch (event) {
        case APP_USBD_CCID_USER_EVT_RX_DONE:
#if NRF_LOG_HEXDUMP_DEBUG_BUFFER
            NRF_LOG_DEBUG("usbd_ccid_data_frame_received(%d bytes):", app_usbd_ccid_ep_output_buffer_size());
            NRF_LOG_HEXDUMP_DEBUG(app_usbd_ccid_ep_output_buffer(), app_usbd_ccid_ep_output_buffer_size());
#endif
            // 受信フレームデータを処理
            ccid_data_frame_received(
                app_usbd_ccid_ep_output_buffer(), 
                app_usbd_ccid_ep_output_buffer_size());
            break;
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

    // モジュール変数を初期化
    ccid_initialize_value();
    NRF_LOG_DEBUG("usbd_ccid_init() done");
}

void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size)
{
    app_usbd_ccid_ep_input_from_buffer(p_data, size);

#if NRF_LOG_HEXDUMP_DEBUG_BUFFER
    NRF_LOG_DEBUG("usbd_ccid_send_data_frame(%d bytes)", size);
    NRF_LOG_HEXDUMP_DEBUG(p_data, size);
#endif
}
