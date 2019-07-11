import hid
import time
import binascii

hid_dev = None

for d in hid.enumerate(0, 0):
    keys = d.keys()
    keys.sort()
    for key in keys:
        if key != 'usage_page':
            continue
        if d[key] != 65451:
            continue

        # Retrieve path for U2F hid interface
        hid_dev = d
        print "HID device: path=%s, usage_page=%d, usage=%d" % (hid_dev['path'], hid_dev['usage_page'], hid_dev['usage'])
        break

if hid_dev is None:
    exit()

'''
HID device: path=USB_1234_0006_0x7fc742407130, usage_page=65451, usage=512
'''
VENDOR_ID  = 1234
PRODUCT_ID = 0006

try:
    print "Opening device"
    h = hid.device(VENDOR_ID, PRODUCT_ID, path=hid_dev['path'])

    # send 8 bytes
    data = [0] * 8
    data[0]  = 0xff
    data[1]  = 0xff
    data[2]  = 0xff
    data[3]  = 0xff
    data[4]  = 0x86
    data[5]  = 0x00
    data[6]  = 0x08
    data[7]  = 0xd4

    h.write(data)
    print "---- sent data ----"
    data_ = bytearray(data)
    print binascii.hexlify(data_)
    print "----"
    print "hid.write done."

    # receive 64 bytes
    rcv = h.read(8)
    print "---- received data ----"
    data_ = bytearray(rcv)
    print binascii.hexlify(data_)
    print "----"
    print "hid.read done."

    print "Closing device"
    h.close()

except IOError, ex:
    print ex
    print "You probably don't have the hard coded test hid. Update the hid.device line"
    print "in this script with one from the enumeration list output above and try again."

print "Done"




