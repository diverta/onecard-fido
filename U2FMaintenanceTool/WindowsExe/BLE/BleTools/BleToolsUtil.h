#ifndef _BLE_TOOLS_UTIL_H_
#define _BLE_TOOLS_UTIL_H_

extern int BleToolsUtil_base64Decode(const char* src, size_t src_len, unsigned char* dest);

// for debug
extern void BleToolsUtil_outputLog(const char *msg);
extern void BleToolsUtil_outputDumpLog(unsigned char *bin, size_t length);

#endif /* _BLE_TOOLS_UTIL_H_ */
