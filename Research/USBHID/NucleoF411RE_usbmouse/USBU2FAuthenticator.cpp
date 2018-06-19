#include "stdint.h"
#include "USBU2FAuthenticator.h"

uint8_t *USBU2FAuthenticator::reportDesc() {

    static uint8_t reportDescriptor[] = 
    {
        0x06, 0xd0, 0xf1,   // USAGE_PAGE (FIDO Alliance)
        0x09, 0x01,         // USAGE (FIDO U2F HID)
        0xa1, 0x01,         // COLLECTION (Application)
        0x09, 0x20,         //   USAGE (Input Report Data)
        0x15, 0x00,         //   LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,   //   LOGICAL_MAXIMUM (255)
        0x75, 0x08,         //   REPORT_SIZE (8)
        0x95, 64,           //   REPORT_COUNT (64)
        0x81, 0x02,         //   INPUT (Data | Absolute | Variable)
        0x09, 0x21,         //   USAGE (Output Report Data)
        0x15, 0x00,         //   LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,   //   LOGICAL_MAXIMUM (255)
        0x75, 0x08,         //   REPORT_SIZE (8)
        0x95, 64,           //   REPORT_COUNT (64)
        0x91, 0x02,         //   OUTPUT (Data | Absolute | Variable)
        0xc0,               // END_COLLECTION
    };
    reportLength = sizeof(reportDescriptor);
    return reportDescriptor;
}

#define DEFAULT_CONFIGURATION (1)
#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

uint8_t *USBU2FAuthenticator::configurationDesc() {
    static uint8_t configurationDescriptor[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,// bLength
        CONFIGURATION_DESCRIPTOR,       // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (MSB)
        0x01,                           // bNumInterfaces
        DEFAULT_CONFIGURATION,          // bConfigurationValue
        0x00,                           // iConfiguration
        C_RESERVED | C_SELF_POWERED,    // bmAttributes
        C_POWER(0),                     // bMaxPowerHello World from Mbed

        INTERFACE_DESCRIPTOR_LENGTH,    // bLength
        INTERFACE_DESCRIPTOR,           // bDescriptorType
        0x00,                           // bInterfaceNumber
        0x00,                           // bAlternateSetting
        0x02,                           // bNumEndpoints (One IN- and one OUT endpoint)
        0x03,                           // bInterfaceClass (HID Class)
        0x00,                           // bInterfaceSubClass (No interface subclass)
        0x00,                           // bInterfaceProtocol (No interface protocol)
        0x00,                           // iInterface

        HID_DESCRIPTOR_LENGTH,          // bLength
        HID_DESCRIPTOR,                 // bDescriptorType
        LSB(0x0111),                    // bcdHID (LSB) HID_VERSION_1_11
        MSB(0x0111),                    // bcdHID (MSB) HID_VERSION_1_11
        0x00,                           // bCountryCode
        0x01,                           // bNumDescriptors
        REPORT_DESCRIPTOR,              // bDescriptorType
        (uint8_t)(LSB(reportDescLength())), // wDescriptorLength (LSB)
        (uint8_t)(MSB(reportDescLength())), // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        0x81,                           // bEndpointAddress (Endpoint 1, IN)
        0x03,                           // bmAttributes     (Interrupt transfer)
        LSB(64),                        // wMaxPacketSize   (LSB)
        MSB(64),                        // wMaxPacketSize   (MSB)
        5,                              // bInterval        (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        0x01,                           // bEndpointAddress (Endpoint 1, OUT)
        0x03,                           // bmAttributes     (Interrupt transfer)
        LSB(64),                        // wMaxPacketSize   (LSB)
        MSB(64),                        // wMaxPacketSize   (MSB)
        5,                              // bInterval        (milliseconds)
    };
    return configurationDescriptor;
}

uint8_t *USBU2FAuthenticator::stringImanufacturerDesc() {
    static uint8_t stringImanufacturerDescriptor[] = {
        26,                 /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        'D',0,
        'i',0,
        'v',0,
        'e',0,
        'r',0,
        't',0,
        'a',0,
        ' ',0,
        'I',0,
        'n',0,
        'c',0, 
        '.',0
    };                      /*bString iManufacturer*/
    return stringImanufacturerDescriptor;
}

uint8_t *USBU2FAuthenticator::stringIproductDesc() {
    static uint8_t stringIproductDescriptor[] = {
        38,                 /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        'U',0,
        '2',0,
        'F',0,
        ' ',0,
        'U',0,
        'S',0,
        'B',0,
        ' ',0,
        'H',0,
        'I',0,
        'D',0,
        ' ',0,
        'D',0,
        'E',0,
        'V',0,
        'I',0,
        'C',0,
        'E',0
    };                      /*bString iProduct*/
    return stringIproductDescriptor;
}

uint8_t * USBU2FAuthenticator::stringIserialDesc() {
    static uint8_t stringIserialDescriptor[] = {
        0x16,               /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '0',0,
        '1',0
    };                      /*bString iSerial*/
    return stringIserialDescriptor;
}
