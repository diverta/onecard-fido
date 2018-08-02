#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

//
// 作業領域
//
static unsigned char requestBuf[128];
static unsigned char responseBuf[1024];
static unsigned char encodedResponse[1024];

static int decodeWebsafeB64String(char *src, unsigned char *dest, size_t destLen)
{
	memset(dest, 0x00, destLen);
	int len = BleToolsUtil_base64Decode(src, strlen(src), dest);
	return len;
}

bool BleU2FHidHelper_ProcessXferMessage(char *recv_hid_message, pBleDevice dev)
{
	// U2F Helperと標準入出力によりやりとりを行う
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage start");

	// 転送されたメッセージをバイト配列に変換
	decodeWebsafeB64String(recv_hid_message, requestBuf, sizeof(requestBuf));
	// メッセージ・ヘッダーから、バイト配列の正しい長さを取得（APDU長＋３）
	int requestLen = requestBuf[1] * 256 + requestBuf[2] + 3;

	// ログ出力
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message from U2F Helper");
	BleToolsUtil_outputDumpLog(requestBuf, requestLen);

	//
	// TODO 取得したメッセージをBLEへリクエスト
	//

	// 
	// TODO BLEからのレスポンスデータをバイト配列に展開
	//

	// これは仮コードです。
	for (int i = 0; i < requestLen; i++) {
		responseBuf[i] = requestBuf[i];
	}
	// メッセージ・ヘッダーから、バイト配列の正しい長さを取得（APDU長＋３）
	int responseLen = responseBuf[1] * 256 + responseBuf[2] + 3;

	// ログ出力
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message back to U2F Helper");
	BleToolsUtil_outputDumpLog(responseBuf, responseLen);

	// レスポンスデータのバイト配列を
	// web-save base64文字列に変換し、U2F Helperに転送
	if (BleToolsUtil_base64Encode((char *)responseBuf, requestLen, encodedResponse) > 0) {
		std::cout << encodedResponse << std::endl;
	}

	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage end");
	return true;
}
