/* 
 * File:   usbd_service_bos.c
 * Author: makmorit
 *
 * Created on 2020/06/25, 12:36
 */
#include "sdk_common.h"
#include "app_usbd_core.h"

// for logging informations
#define NRF_LOG_MODULE_NAME usbd_service_bos
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();

static const uint8_t USBD_FS_BOSDesc[] = {
    0x05,                   // bLength
    0x0f,                   // bDescriptorType
    0x21, 0x00,             // total length
    0x01,                   // Number of device capabilities

    // Microsoft OS 2.0 Platform Capability Descriptor 
    // MS_VendorCode == 0x02
    0x1C,                   // bLength
    0x10,                   // Device Capability descriptor
    0x05,                   // Platform Capability descriptor
    0x00,                   // Reserved
    0xDF, 0x60, 0xDD, 0xD8, // MS OS 2.0 GUID
    0x89, 0x45, 0xC7, 0x4C,
    0x9C, 0xD2, 0x65, 0x9D,
    0x9E, 0x64, 0x8A, 0x9F,
    0x00, 0x00, 0x03, 0x06, // Windows version
    0xB2, 0x00,             // Descriptor set length
    0x02,                   // Vendor request code
    0x00                    // Alternate enumeration code
};

static const uint8_t USBD_FS_MSOS20Desc[] = {
    // Microsoft OS 2.0 descriptor set header (table 10)
    0x0A, 0x00,             // Descriptor size (10 bytes)
    0x00, 0x00,             // MS OS 2.0 descriptor set header
    0x00, 0x00, 0x03, 0x06, // Windows version (8.1) (0x06030000)
    0xB2, 0x00,             // Size, MS OS 2.0 descriptor set

    // Microsoft OS 2.0 configuration subset header
    0x08, 0x00,             // Descriptor size (8 bytes)
    0x01, 0x00,             // MS OS 2.0 configuration subset header
    0x00,                   // bConfigurationValue
    0x00,                   // Reserved
    0xA8, 0x00,             // Size, MS OS 2.0 configuration subset

    // Microsoft OS 2.0 function subset header
    0x08, 0x00,             // Descriptor size (8 bytes)
    0x02, 0x00,             // MS OS 2.0 function subset header
    0x01,                   // First interface number
    0x00,                   // Reserved
    0xA0, 0x00,             // Size, MS OS 2.0 function subset

    // Microsoft OS 2.0 compatible ID descriptor (table 13)
    0x14, 0x00,             // wLength
    0x03, 0x00,             // MS_OS_20_FEATURE_COMPATIBLE_ID
    'W',  'I',  'N',  'U',  'S',  'B',  0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    0x84, 0x00,             // wLength:
    0x04, 0x00,             // wDescriptorType: MS_OS_20_FEATURE_REG_PROPERTY: 0x04 (Table 9)
    0x07, 0x00,             // wPropertyDataType: REG_MULTI_SZ (Table 15)
    0x2A, 0x00,             // wPropertyNameLength:
    // bPropertyName: "DeviceInterfaceGUID"
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00,
    'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00,
    'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    0x50, 0x00,             // wPropertyDataLength
    // bPropertyData: "{244eb29e-e090-4e49-81fe-1f20f8d3b8f4}"
    '{', 0x00, '2', 0x00, '4', 0x00, '4', 0x00, 'E', 0x00, 'B', 0x00, '2', 0x00,
    '9', 0x00, 'E', 0x00, '-', 0x00, 'E', 0x00, '0', 0x00, '9', 0x00, '0', 0x00,
    '-', 0x00, '4', 0x00, 'E', 0x00, '4', 0x00, '9', 0x00, '-', 0x00, '8', 0x00,
    '1', 0x00, 'F', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, 'F', 0x00, '2', 0x00,
    '0', 0x00, 'F', 0x00, '8', 0x00, 'D', 0x00, '3', 0x00, 'B', 0x00, '8', 0x00,
    'F', 0x00, '4', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};

static ret_code_t setup_req_std_in(app_usbd_setup_evt_t const *p_setup_ev)
{
    app_usbd_setup_reqrec_t req_rec = app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType);
    NRF_LOG_DEBUG("setup_req_std_in: bmRequestType=%u, bRequest=%u", req_rec, p_setup_ev->setup.bRequest);
    
    // Only Get Descriptor standard IN request is supported by CCID class
    if ((req_rec == APP_USBD_SETUP_REQREC_DEVICE) &&
        (p_setup_ev->setup.bRequest == APP_USBD_SETUP_STDREQ_GET_DESCRIPTOR)) {
        NRF_LOG_DEBUG("Request for get descriptor: type=%02x, index=%02x", 
            p_setup_ev->setup.wValue.hb, p_setup_ev->setup.wValue.lb);

        if (p_setup_ev->setup.wValue.hb == 0x0f) {
            size_t dsc_len = sizeof(USBD_FS_BOSDesc);
            return app_usbd_core_setup_rsp(&(p_setup_ev->setup), USBD_FS_BOSDesc, dsc_len);
        }  
    }

    NRF_LOG_DEBUG("Unable to find descriptor");
    return NRF_ERROR_NOT_SUPPORTED;
}

static ret_code_t setup_req_vendor_in(app_usbd_setup_evt_t const *p_setup_ev)
{
    app_usbd_setup_reqrec_t req_rec = app_usbd_setup_req_rec(p_setup_ev->setup.bmRequestType);
    NRF_LOG_DEBUG("setup_req_vendor_in: bmRequestType=%u, bRequest=%u", req_rec, p_setup_ev->setup.bRequest);

    // MS OS 2.0
    if ((req_rec == APP_USBD_SETUP_REQREC_DEVICE) && (p_setup_ev->setup.bRequest == 0x02)) {
        NRF_LOG_DEBUG("Request for get descriptor: index=%04x", p_setup_ev->setup.wIndex.w);

        if (p_setup_ev->setup.wIndex.w == 0x0007) {
            size_t dsc_len = sizeof(USBD_FS_MSOS20Desc);
            return app_usbd_core_setup_rsp(&(p_setup_ev->setup), USBD_FS_MSOS20Desc, dsc_len);
        }
    }

    NRF_LOG_DEBUG("Unable to find descriptor");
    return NRF_ERROR_NOT_SUPPORTED;
}

uint32_t usbd_service_bos_setup(void const *p_ev)
{
    uint32_t ret = NRF_SUCCESS;
    app_usbd_setup_evt_t const *p_setup_ev = (app_usbd_setup_evt_t const *)p_ev;

    if (app_usbd_setup_req_dir(p_setup_ev->setup.bmRequestType) == APP_USBD_SETUP_REQDIR_IN) {
        switch (app_usbd_setup_req_typ(p_setup_ev->setup.bmRequestType)) {
            case APP_USBD_SETUP_REQTYPE_STD:
                setup_req_std_in(p_setup_ev);
                break;
            case APP_USBD_SETUP_REQTYPE_CLASS:
                NRF_LOG_DEBUG("setup_req_class_in");
                break;
            case APP_USBD_SETUP_REQTYPE_VENDOR:
                setup_req_vendor_in(p_setup_ev);
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
