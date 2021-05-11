/* 
 * File:   app_usb_ccid_define.h
 * Author: makmorit
 *
 * Created on 2021/05/11, 16:26
 */
#ifndef APP_USB_CCID_DEFINE_H
#define APP_USB_CCID_DEFINE_H

#include <stdbool.h>
#include <stdint.h>
#include <usb/usb_common.h>

#ifdef __cplusplus
extern "C" {
#endif

//
// CCID関連定義
//
#define APDU_BUFFER_SIZE            1280
#define APDU_DATA_SIZE              (APDU_BUFFER_SIZE + 2)
#define CCID_CMD_HEADER_SIZE        10
#define CCID_NUMBER_OF_SLOTS        1

#define CCID_CLASS                  0x0b
#define CCID_SUBCLASS_NO_BOOT       0x00
#define CCID_PROTOCOL               0x00

#define CCID_IN_EP_ADDR             0x81
#define CCID_OUT_EP_ADDR            0x01
#define CCID_BULK_EP_MPS            64

//
// CCID I/F用デスクリプター
//
struct usb_ccid_descriptor {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdCCID;
    uint8_t bMaxSlotIndex;
    uint8_t bVoltageSupport;
    uint32_t dwProtocols;
    uint32_t dwDefaultClock;
    uint32_t dwMaximumClock;
    uint8_t bNumClockSupported;
    uint32_t dwDataRate;
    uint32_t dwMaxDataRate;
    uint8_t bNumDataRatesSupported;
    uint32_t dwMaxIFSD;
    uint32_t dwSynchProtocols;
    uint32_t dwMechanical;
    uint32_t dwFeatures;
    uint32_t dwMaxCCIDMessageLength;
    uint8_t bClassGetResponse;
    uint8_t bClassEnvelope;
    uint16_t wLcdLayout;
    uint8_t bPINSupport;
    uint8_t bMaxCCIDBusySlots;
} __packed;

struct usb_ccid_config {
    struct usb_if_descriptor if0;
    struct usb_ccid_descriptor if0_ccid_desc;
    struct usb_ep_descriptor if0_in_ep;
    struct usb_ep_descriptor if0_out_ep;
} __packed;

#ifdef __cplusplus
}
#endif

#endif /* APP_USB_CCID_DEFINE_H */
