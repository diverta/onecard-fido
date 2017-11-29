#ifndef _BLE_TOOLS_H_
#define _BLE_TOOLS_H_

#include "BleApi.h"

extern char *arg_DeviceIdentifier;

extern int  BleTools_ProcessCommand(BleApiConfiguration &configuration, pBleDevice dev);
extern int  BleTools_ParseArguments(int argc, char *argv[], BleApiConfiguration &configuration);


#endif /* _BLE_TOOLS_H_ */
