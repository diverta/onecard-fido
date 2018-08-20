#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

//
// 作業領域
//
static unsigned char requestBuf[256];
static unsigned char responseBuf[1024];
static unsigned char encodedResponse[1024];

//
// BLEデバイスからのレスポンスを保持
//
static unsigned char reply[1024];
static unsigned char replyCmd;
static size_t        replyLength;
static uint16_t      replyStatusWord;

static void xferBleU2fRequest(pBleDevice dev)
{
	// メッセージ・ヘッダーから、バイト配列の正しい長さを取得（APDU長＋３）

	// コマンドはヘッダーの 1バイト目
	unsigned char CMD = requestBuf[0];

	// APDU長はヘッダーの 2, 3バイト目
	size_t apduLen = requestBuf[1] * 256 + requestBuf[2];

	// APDUは転送バイト配列の 4バイト目以降
	unsigned char *apduBuf = &requestBuf[3];

	// レスポンス長を設定
	replyLength = sizeof(reply);

	// ログ出力
	// BleToolsUtil_outputLog("xferBleU2fRequest: APDU to BLE");
	// BleToolsUtil_outputDumpLog(apduBuf, apduLen);

	// BLEリクエストを送信 ---> レスポンスを受信
	memset(reply, 0x00, sizeof(reply));
	ReturnValue retval = dev->CommandWrite(
		CMD, apduBuf, apduLen, &replyCmd, reply, &replyLength);

	// 受信に失敗した場合
	if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		// ERRORフレームを生成
		//  CMD:   0xbf (ERROR)
		//  VALUE: 0x7f (ERR_OTHER)
		responseBuf[0] = 0xbf;
		responseBuf[1] = 0;
		responseBuf[2] = 1;
		responseBuf[3] = 0x7f;

		// ログ出力
		BleToolsUtil_outputLog("xferBleU2fRequest: Command write failed");
		return;
	}

	// ログ出力
	// BleToolsUtil_outputLog("xferBleU2fRequest: APDU from BLE");
	// BleToolsUtil_outputDumpLog(reply, replyLength);

	// レスポンスデータにヘッダー、APDUの順でコピー
	responseBuf[0] = replyCmd;
	responseBuf[1] = (unsigned char)(replyLength / 256);
	responseBuf[2] = replyLength % 256;
	for (size_t i = 0; i < replyLength; i++) {
		responseBuf[3 + i] = reply[i];
	}
}

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

	// 取得したメッセージをBLEへリクエスト
	//  BLEからのレスポンスデータは、
	//  ヘッダーとAPDUを同一のバイト配列に格納
	xferBleU2fRequest(dev);

	// メッセージ・ヘッダーから、バイト配列の正しい長さを取得（APDU長＋３）
	int responseLen = responseBuf[1] * 256 + responseBuf[2] + 3;

	// ログ出力
	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage: Message back to U2F Helper");
	BleToolsUtil_outputDumpLog(responseBuf, responseLen);

	// レスポンスデータのバイト配列を
	// web-save base64文字列に変換し、U2F Helperに転送
	if (BleToolsUtil_base64Encode((char *)responseBuf, responseLen, encodedResponse) > 0) {
		std::cout << encodedResponse << std::endl;
	}

	BleToolsUtil_outputLog("BleU2FHidHelper_ProcessXferMessage end");
	return true;
}
