#ifndef _BLE_U2F_HID_HELPER_H_
#define _BLE_U2F_HID_HELPER_H_

#include "BleApi.h"

extern bool BleU2FHidHelper_ProcessXferMessage(char *recv_hid_message, pBleDevice dev);

#endif /* _BLE_U2F_HID_HELPER_H_ */
