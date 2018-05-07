#ifndef _BLE_TOOLS_UTIL_H_
#define _BLE_TOOLS_UTIL_H_

extern int BleToolsUtil_base64Decode(const char* src, size_t src_len, unsigned char* dest);
extern int BleToolsUtil_base64Encode(const char* src, size_t src_len, unsigned char* dest);

// for checking status word
extern bool        BleToolsUtil_checkStatusWord(unsigned char *request, unsigned char *reply, size_t replyLength);
extern const char *BleToolsUtil_checkStatusWordMessage(void);
extern void        BleToolsUtil_checkStatusWordMessagePrint(const char *functionName);

// for debug
extern void BleToolsUtil_outputLog(const char *msg);
extern void BleToolsUtil_outputDumpLog(unsigned char *bin, size_t length);

#endif /* _BLE_TOOLS_UTIL_H_ */
