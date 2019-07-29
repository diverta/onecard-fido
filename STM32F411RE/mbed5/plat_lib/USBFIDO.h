#ifndef USB_FIDO_H
#define USB_FIDO_H

// from mbed OS USBDEVICE Library
#include "USBHID.h"

class USBFIDO: public USBHID
{
    public:
        //
        // Constructor
        //
        USBFIDO(uint8_t output_report_length, uint8_t input_report_length, uint16_t vendor_id=0xf055, uint16_t product_id=0x0001, uint16_t product_release=0x0001)
        : USBHID(0, 0, vendor_id, product_id, product_release, false) {
            output_length = output_report_length;
            input_length = input_report_length;
            send_report.length = input_length;
            connect();
        };

        //
        // To define the report descriptor. 
        //
        virtual uint8_t * reportDesc();
        //
        // Get string manufacturer/product descriptor
        //
        virtual uint8_t * stringImanufacturerDesc();
        virtual uint8_t * stringIproductDesc();
        //
        // Get string serial descriptor
        //
        virtual uint8_t * stringIserialDesc();

        uint8_t output_length;
        uint8_t input_length;

        // This report will contain data to be sent
        HID_REPORT send_report;
        HID_REPORT recv_report;

    protected:
        //
        // Get configuration descriptor
        //
        virtual uint8_t * configurationDesc();
};

#endif
