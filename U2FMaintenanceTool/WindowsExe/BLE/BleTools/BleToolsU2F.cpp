#include <stdio.h>

#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

//
// BLEデバイスに対するリクエストを保持
//
static unsigned char request[U2F_REQUEST_BUF_SIZE];
static size_t        requestLength = U2F_REQUEST_BUF_SIZE;

//
// BLEデバイスからのレスポンスを保持
//
static unsigned char reply[U2F_REPLY_BUF_SIZE];
static size_t        replyLength = U2F_REPLY_BUF_SIZE;
static uint16_t      replyStatusWord;

static bool sendBleU2fRequest(pBleDevice dev)
{
	// ログ出力
	BleToolsUtil_outputLog("sendBleU2fRequest: Message to BLE");
	BleToolsUtil_outputDumpLog(request, requestLength);

	unsigned char replyCmd;
	memset(reply, 0x00, sizeof(reply));
	ReturnValue retval = dev->CommandWrite(FIDO_BLE_CMD_MSG, request, requestLength,
		&replyCmd, reply, &replyLength);
	if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		BleToolsUtil_outputLog("sendBleU2fRequest: Command write failed");
		return false;
	}

	// ログ出力
	BleToolsUtil_outputLog("sendBleU2fRequest: Message from BLE");
	BleToolsUtil_outputDumpLog(reply, replyLength);

	// 戻りのコマンドをチェック
	if (replyCmd != FIDO_BLE_CMD_MSG) {
		BleToolsUtil_outputLog("sendBleU2fRequest: reply command != FIDO_BLE_CMD_MSG");
		return false;
	}

	// 戻りのステータスワードをチェック
	if (BleToolsUtil_checkStatusWord(request, reply, replyLength) == false) {
		BleToolsUtil_outputLog("sendBleU2fRequest: status word != FIDO_RESP_SUCCESS");
		BleToolsUtil_outputLog(BleToolsUtil_checkStatusWordMessage());
		return false;
	}

	// ログ出力
	BleToolsUtil_outputLog("sendBleU2fRequest: Message from BLE");
	BleToolsUtil_outputDumpLog(reply, replyLength);

	return true;
}

bool sendU2FRegisterRequest(pBleDevice dev,
	const unsigned char *challengeBuf, const unsigned char *appIdBuf)
{
	size_t pos = 0;

	// リクエストデータを配列にセット
	request[0] = 0x00;
	request[1] = U2F_INS_REGISTER;
	request[2] = 0x00;
	request[3] = 0x00;
	request[4] = 0x00;
	request[5] = 0x00;
	request[6] = U2F_NONCE_SIZE + U2F_APPID_SIZE;

	// challengeを設定
	pos = 7;
	for (size_t i = 0; i < U2F_NONCE_SIZE; i++) {
		request[pos + i] = challengeBuf[i];
	}
	pos += U2F_NONCE_SIZE;

	// appIdを設定
	for (size_t i = 0; i < U2F_APPID_SIZE; i++) {
		request[pos + i] = appIdBuf[i];
	}
	pos += U2F_APPID_SIZE;

	// Leを設定
	request[pos++] = 0x00;
	request[pos++] = 0x00;

	// BLE U2Fリクエストを転送
	requestLength = pos;
	if (sendBleU2fRequest(dev) == false) {
		return false;
	}

	return true;
}

bool sendU2FAuthenticateRequest(pBleDevice dev,
	const unsigned char *challengeBuf, const unsigned char *appIdBuf,
	const unsigned char *keyHandleBuf)
{
	size_t pos = 0;

	// リクエストデータを配列にセット
	request[0] = 0x00;
	request[1] = U2F_INS_AUTHENTICATE;
	request[2] = U2F_AUTH_ENFORCE;
	request[3] = 0x00;
	request[4] = 0x00;
	request[5] = 0x00;
	request[6] = U2F_NONCE_SIZE + U2F_APPID_SIZE + U2F_KEYHANDLE_SIZE + 1;

	// challengeを設定
	pos = 7;
	for (size_t i = 0; i < U2F_NONCE_SIZE; i++) {
		request[pos + i] = challengeBuf[i];
	}
	pos += U2F_NONCE_SIZE;

	// appIdを設定
	for (size_t i = 0; i < U2F_APPID_SIZE; i++) {
		request[pos + i] = appIdBuf[i];
	}
	pos += U2F_APPID_SIZE;

	// キーハンドル長を設定
	request[pos++] = U2F_KEYHANDLE_SIZE;

	// キーハンドルを設定
	for (size_t i = 0; i < U2F_KEYHANDLE_SIZE; i++) {
		request[pos + i] = keyHandleBuf[i];
	}
	pos += U2F_KEYHANDLE_SIZE;

	// Leを設定
	request[pos++] = 0x00;
	request[pos++] = 0x00;

	// BLE U2Fリクエストを転送
	requestLength = pos;
	if (sendBleU2fRequest(dev) == false) {
		return false;
	}

	return true;
}

//
// レスポンスに対する処理
//
uint16_t BleToolsU2F_replyStatusWord(void)
{
	// BLEレスポンスのステータスワードを戻す
	return replyStatusWord;
}

void BleToolsU2F_encodeB64Reply(unsigned char *encodedBuf, size_t encodedBufLength) 
{
	// BLEレスポンスから
	// ステータスワード（末尾２バイト）を除去し、
	// Web Safe Base64エンコードして戻す
	memset((char *)encodedBuf, 0x00, encodedBufLength);
	int len = BleToolsUtil_base64Encode((char *)reply, replyLength - 2, encodedBuf);

	// ログ出力
	char buf[64];
	sprintf_s(buf, "BleToolsU2F_encodeB64Reply: encoded length=%d", len);
	BleToolsUtil_outputLog(buf);
	BleToolsUtil_outputLog((char *)encodedBuf);
}

//
// ヘルスチェック機能
//
// Register時のappId、キーハンドルを保持
static unsigned char keyHandleForCheck[U2F_KEYHANDLE_SIZE];
static unsigned char appIdForCheck[U2F_APPID_SIZE];
static unsigned char nonceForCheck[U2F_NONCE_SIZE];

static bool processTestRegister(pBleDevice dev)
{
	// challengeはランダム値を設定
	for (size_t i = 0; i < U2F_NONCE_SIZE; i++) {
		nonceForCheck[i] = (rand() & 0xFF);
	}

	// appIdはランダム値を設定
	// (Authenticate時に備え、appIdForCheckに保持しておく)
	for (size_t i = 0; i < U2F_APPID_SIZE; i++) {
		appIdForCheck[i] = (rand() & 0xFF);
	}

	// BLE U2Fリクエストを転送
	if (sendU2FRegisterRequest(dev, nonceForCheck, appIdForCheck) == false) {
		// エラーメッセージがあれば画面表示
		BleToolsUtil_checkStatusWordMessagePrint("sendU2FRegisterRequest");
		return false;
	}

	// Registerレスポンスからキーハンドル
	// (68バイト目から64バイト)を切り出して保持
	memcpy(keyHandleForCheck, reply + 67, U2F_KEYHANDLE_SIZE);

	return true;
}

static bool processTestAuthenticate(pBleDevice dev)
{
	// challengeはランダム値を設定
	// appIdはRegisterリクエスト時と同値を設定
	// キーハンドルはRegisterレスポンスと同値を設定
	for (size_t i = 0; i < U2F_NONCE_SIZE; i++) {
		nonceForCheck[i] = (rand() & 0xFF);
	}

	// BLE U2Fリクエストを転送
	if (sendU2FAuthenticateRequest(dev, nonceForCheck, appIdForCheck, keyHandleForCheck) == false) {
		// エラーメッセージがあれば画面表示
		BleToolsUtil_checkStatusWordMessagePrint("sendU2FAuthenticateRequest");
		return false;
	}

	return true;
}

bool BleToolsU2F_healthCheck(pBleDevice dev)
{
	std::cout << "ヘルスチェックを開始します。" << std::endl;

	// U2F Registerを実行
	if (processTestRegister(dev) == false) {
		return false;
	}
	std::cout << "U2F Registerが成功しました。" << std::endl;

	// U2F Authenticateを実行
	if (processTestAuthenticate(dev) == false) {
		return false;
	}
	std::cout << "U2F Authenticateが成功しました。" << std::endl;

	return true;
}
