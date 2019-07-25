#include "stdint.h"
#include "USBDHIDService.h"

uint8_t *USBDHIDService::reportDesc() 
{
    static uint8_t reportDescriptor[] = 
    {
        0x06, 0xd0, 0xf1, /* Usage Page (FIDO Alliance),         */
        0x09, 0x01,       /* Usage (FIDO USB HID),               */
        0xa1, 0x01,       /*  Collection (Application),          */
        0x09, 0x20,       /*   Usage (Input Report Data),        */
        0x15, 0x00,       /*   Logical Minimum (0),              */
        0x26, 0xff, 0x00, /*   Logical Maximum (255),            */
        0x75, 0x08,       /*   Report Size (8),                  */
        0x95, 64,         /*   Report Count (64),                */
        0x81, 0x02,       /*   Input (Data, Variable, Absolute)  */
        0x09, 0x21,       /*   Usage (Output Report Data),       */
        0x15, 0x00,       /*   Logical Minimum (0),              */
        0x26, 0xff, 0x00, /*   Logical Maximum (255),            */
        0x75, 0x08,       /*   Report Size (8),                  */
        0x95, 64,         /*   Report Count (64),                */
        0x91, 0x02,       /*   Output (Data, Variable, Absolute) */
        0xc0,             /* End Collection                      */
    };
    reportLength = sizeof(reportDescriptor);
    return reportDescriptor;
}

#define TOTAL_DESCRIPTOR_LENGTH ((1 * CONFIGURATION_DESCRIPTOR_LENGTH) \
                               + (1 * INTERFACE_DESCRIPTOR_LENGTH) \
                               + (1 * HID_DESCRIPTOR_LENGTH) \
                               + (2 * ENDPOINT_DESCRIPTOR_LENGTH))

uint8_t *USBDHIDService::configurationDesc() 
{
    static uint8_t configurationDescriptor[] = {
        CONFIGURATION_DESCRIPTOR_LENGTH,// bLength
        CONFIGURATION_DESCRIPTOR,       // bDescriptorType
        LSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (LSB)
        MSB(TOTAL_DESCRIPTOR_LENGTH),   // wTotalLength (MSB)
        0x01,                           // bNumInterfaces
        0x01,                           // bConfigurationValue (DEFAULT_CONFIGURATION)
        0x00,                           // iConfiguration
        C_RESERVED,                     // bmAttributes (C_RESERVED | C_SELF_POWERED)
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
        0x22,                           // bDescriptorType (USB_HID_REPORT_DESCRIPTOR)
        (uint8_t)(LSB(reportDescLength())), // wDescriptorLength (LSB)
        (uint8_t)(MSB(reportDescLength())), // wDescriptorLength (MSB)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        0x01,                           // bEndpointAddress (Endpoint 1, OUT)
        0x03,                           // bmAttributes     (Interrupt transfer)
        LSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize   (LSB)
        MSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize   (MSB)
        1,                              // bInterval        (milliseconds)

        ENDPOINT_DESCRIPTOR_LENGTH,     // bLength
        ENDPOINT_DESCRIPTOR,            // bDescriptorType
        0x81,                           // bEndpointAddress (Endpoint 1, IN)
        0x03,                           // bmAttributes     (Interrupt transfer)
        LSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize   (LSB)
        MSB(MAX_PACKET_SIZE_EPINT),     // wMaxPacketSize   (MSB)
        1,                              // bInterval        (milliseconds)
    };
    return configurationDescriptor;
}

uint8_t *USBDHIDService::stringImanufacturerDesc() 
{
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

uint8_t *USBDHIDService::stringIproductDesc() 
{
    static uint8_t stringIproductDescriptor[] = {
        40,                 /*bLength*/
        STRING_DESCRIPTOR,  /*bDescriptorType 0x03*/
        'F',0,
        'I',0,
        'D',0,
        'O',0,
        '2',0,
        ' ',0,
        'A',0,
        'U',0,
        'T',0,
        'H',0,
        'E',0,
        'N',0,
        'T',0,
        'I',0,
        'C',0,
        'A',0,
        'T',0,
        'O',0,
        'R',0
    };                      /*bString iProduct*/
    return stringIproductDescriptor;
}

uint8_t * USBDHIDService::stringIserialDesc() 
{
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

void USBDHIDService::doInitialize() 
{
    printf("USBDHIDService start\r\n");
    
    send_report.length = 64;
}

bool USBDHIDService::doProcess() 
{
    // HIDデータフレームを受信
    if (read(&recv_report)) {
        printf("recv:");
        for (int i = 0; i < recv_report.length; i++) {
            if (i % 16 == 0) {
                printf("\r\n");
            }
            printf("%02x ", recv_report.data[i]);

            // 受信メッセージを送信メッセージ領域にコピー
            send_report.data[i] = recv_report.data[i];
        }
        printf("\r\n");

        // HIDデータフレームを送信
        // 受信メッセージをecho back
        sendNB(&send_report);
        printf("sent: done.\r\n");
    }

    wait(0.01);
    return true;
}
