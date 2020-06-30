import hid
import time
import binascii

hid_dev = None

for d in hid.enumerate(0, 0):
    keys = d.keys()
    for key in keys:
        if key != 'usage_page':
            continue
        if d[key] != 61904:
            continue

        # Retrieve path for U2F hid interface
        hid_dev = d
        print("HID device: path=%s, usage_page=%d, usage=%d" % (hid_dev['path'], hid_dev['usage_page'], hid_dev['usage']))
        break

if hid_dev is None:
    exit()

'''
manufacturer_string : Divert
path : USB_f055_0001_0x7ffc72c070f0
product_id : 1
product_string : U2F USB H
release_number : 1
serial_number : 00000
usage : 1
usage_page : 65280
vendor_id : 61525
'''
VENDOR_ID  = 61525
PRODUCT_ID = 1

try:
    h = hid.Device(VENDOR_ID, PRODUCT_ID, path=hid_dev['path'])

    # send 64 bytes
    data = [0] * 64
    data[0]  = 0xff
    data[1]  = 0xff
    data[2]  = 0xff
    data[3]  = 0xff
    data[4]  = 0x86
    data[5]  = 0x00
    data[6]  = 0x08
    data[7]  = 0xd4
    data[8]  = 0xe5
    data[9]  = 0xf6
    data[10] = 0x07
    data[11] = 0x18
    data[12] = 0x29
    data[13] = 0x30
    data[14] = 0x41

    h.write(bytes(data))
    print("---- sent data ----")
    data_ = bytearray(data)
    print(binascii.hexlify(data_[:32]))
    print(binascii.hexlify(data_[32:]))

    # receive 64 bytes
    rcv = h.read(64)
    print("---- received data ----")
    data_ = bytearray(rcv)
    print(binascii.hexlify(data_[:32]))
    print(binascii.hexlify(data_[32:]))
    print("---- INIT done ----")


    # send 64 bytes
    data = [0] * 64
    data[0]  = data_[15]
    data[1]  = data_[16]
    data[2]  = data_[17]
    data[3]  = data_[18]
    data[4]  = 0xc5

    h.write(bytes(data))
    print("---- sent data ----")
    data_ = bytearray(data)
    print(binascii.hexlify(data_[:32]))
    print(binascii.hexlify(data_[32:]))

    # receive 64 bytes
    rcv = h.read(64)
    print("---- received data ----")
    data_ = bytearray(rcv)
    print(binascii.hexlify(data_[:32]))
    print(binascii.hexlify(data_[32:]))
    print("---- command done ----")

    h.close()

except IOError as ex:
    print(ex)
    print("You probably don't have the hard coded test hid. Update the hid.device line")
    print("in this script with one from the enumeration list output above and try again.")
