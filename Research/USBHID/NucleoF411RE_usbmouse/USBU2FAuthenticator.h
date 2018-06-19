#ifndef USBU2FAUTHENTICATOR_H
#define USBU2FAUTHENTICATOR_H

#include "USBHID.h"

class USBU2FAuthenticator: public USBHID
{
    public:
        //
        // Constructor
        //
        USBU2FAuthenticator(bool debug, uint16_t vendor_id=0xf055, uint16_t product_id=0x0001, uint16_t product_release=0x0001)
        : USBHID(0, 0, vendor_id, product_id, product_release, false)
        {
            this->debug = debug;
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

    protected:
        //
        // Get configuration descriptor
        //
        virtual uint8_t * configurationDesc();

    private:
        bool debug;
};

#endif
