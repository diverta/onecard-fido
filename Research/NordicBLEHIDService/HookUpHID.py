import hid
import time

'''
for d in hid.enumerate(0, 0):
    keys = d.keys()
    keys.sort()
    for key in keys:
        print "%s : %s" % (key, d[key])
    print ""
'''


'''
manufacturer_string : 
path : Bluetooth Low Energy_1915_eeee_0x7fd62d41fb20
product_id : 61166
product_string : Nordic_K
release_number : 1
serial_number : 
usage : 1
usage_page : 61904
vendor_id : 6421
'''
VENDOR_ID  = 6421
PRODUCT_ID = 61166

try:
    print "Opening device"
    h = hid.device(VENDOR_ID, PRODUCT_ID)

    # send 10 bytes
    data = [0] * 10
    data[0]  = 0x00
    data[1]  = 0xff
    data[2]  = 0xa1
    data[3]  = 0xb2
    data[4]  = 0xc3
    data[5]  = 0xd4
    data[6]  = 0xe5
    data[7]  = 0xf6
    data[8]  = 0x07
    data[9]  = 0x18

    h.write(data)
    print "hid.write done."
    print data

    # receive 8 bytes
    rcv = h.read(8)
    print "hid.read done."
    print rcv
    '''
    '''
    # send 10 bytes
    data = [0] * 10
    data[0]  = 0x00
    data[1]  = 0x00
    data[2]  = 0x21
    data[3]  = 0x32
    data[4]  = 0x43
    data[5]  = 0x54
    data[6]  = 0x65
    data[7]  = 0x76
    data[8]  = 0x87
    data[9]  = 0x98

    h.write(data)
    print "hid.write done."
    print data

    # receive 8 bytes
    rcv = h.read(8)
    print "hid.read done."
    print rcv


    print "Closing device"
    h.close()

except IOError, ex:
    print ex
    print "You probably don't have the hard coded test hid. Update the hid.device line"
    print "in this script with one from the enumeration list output above and try again."

print "Done"




