#ifndef _BLE_TOOLS_U2F_H_
#define _BLE_TOOLS_U2F_H_

#define U2F_KEYHANDLE_SIZE 64
#define U2F_REQUEST_BUF_SIZE 256
#define U2F_REPLY_BUF_SIZE 2048

extern bool sendU2FRegisterRequest(pBleDevice dev,
	const unsigned char *challengeBuf, const unsigned char *appIdBuf);
extern bool sendU2FAuthenticateRequest(pBleDevice dev,
	const unsigned char *challengeBuf, const unsigned char *appIdBuf,
	const unsigned char *keyHandleBuf);

extern uint16_t BleToolsU2F_replyStatusWord(void);
extern void     BleToolsU2F_encodeB64Reply(unsigned char *encodedBuf, size_t encodedBufLength);
extern bool     BleToolsU2F_healthCheck(pBleDevice dev);

#endif /* _BLE_TOOLS_U2F_H_ */
