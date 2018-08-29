#ifndef USBU2FAUTHENTICATOR_H
#define USBU2FAUTHENTICATOR_H

#include "USBHID.h"

class USBU2FAuthenticator: public USBHID
{
    public:
        //
        // Constructor
        //
        USBU2FAuthenticator(uint8_t output_report_length, uint8_t input_report_length, uint16_t vendor_id=0xf055, uint16_t product_id=0x0001, uint16_t product_release=0x0001)
        : USBHID(0, 0, vendor_id, product_id, product_release, false) {
            output_length = output_report_length;
            input_length = input_report_length;
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

        bool readNB2(HID_REPORT *report);
        bool send2(HID_REPORT *report);

    protected:
        //
        // Get configuration descriptor
        //
        virtual uint8_t * configurationDesc();

    private:
        uint8_t output_length;
        uint8_t input_length;
};

#endif

