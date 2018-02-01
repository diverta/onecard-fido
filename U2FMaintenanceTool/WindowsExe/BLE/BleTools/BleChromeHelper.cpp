#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsUtil.h"

// For JSON parse/serialize
#include "picojson.h"

static U2F_REGISTER_REQ      registerRequest;
static U2F_AUTHENTICATE_REQ  authRequest;

static U2F_REGISTER_RESP     registerResponse;
static U2F_AUTHENTICATE_RESP authResponse;

typedef struct {
	std::string type;
	std::string version;
	std::string challengeHash;
	std::string appIdHash;
	std::string keyHandle;
	uint16_t    statusWordNumber;
	bool enroll;
	bool sign;
	bool empty;
} CHROME_U2F_MESSAGE;
static CHROME_U2F_MESSAGE chromeU2FMessage;

//
// BLEデバイスに対するリクエスト、レスポンスを保持
//
static unsigned char request[256];
static unsigned char reply[2048];
static size_t        replyLength = sizeof(reply);
//
// エンコード、デコード用作業領域
//
static unsigned char encodedBuf[1024];
static unsigned char decodedBuf[128];

static void parseChromeJSONMessage(std::string &json, CHROME_U2F_MESSAGE *msg)
{
	// 標識変数を初期化
	msg->empty  = true;
	msg->sign   = false;
	msg->enroll = false;

	// JSON文字列のparseを実行
	picojson::value v;
	std::string err;
	picojson::parse(v, json.begin(), json.end(), &err);
	if (!err.empty()) {
		return;
	}

	// リクエストタイプを取得
	picojson::object& o = v.get<picojson::object>();
	msg->type = o["type"].get<std::string>();

	// Enroll (U2F Register)
	if (msg->type == std::string("enroll_helper_request")) {
		msg->enroll = true;

		picojson::value&  enrollChallenges_v = o["enrollChallenges"].get(0);
		picojson::object& enrollChallenges_o = enrollChallenges_v.get<picojson::object>();

		msg->version       = enrollChallenges_o["version"].get<std::string>();
		msg->challengeHash = enrollChallenges_o["challengeHash"].get<std::string>();
		msg->appIdHash     = enrollChallenges_o["appIdHash"].get<std::string>();
		msg->empty = false;
	}

	// Sign (U2F Sign)
	if (msg->type == std::string("sign_helper_request")) {
		msg->sign = true;

		picojson::value&  signData_v = o["signData"].get(0);
		picojson::object& signData_o = signData_v.get<picojson::object>();

		msg->version       = signData_o["version"].get<std::string>();
		msg->challengeHash = signData_o["challengeHash"].get<std::string>();
		msg->appIdHash     = signData_o["appIdHash"].get<std::string>();
		msg->keyHandle     = signData_o["keyHandle"].get<std::string>();
		msg->empty = false;
	}
}

static void decodeWebsafeB64EncodeString(std::string &src, unsigned char *dest, size_t destLength)
{
	memset(decodedBuf, 0x00, sizeof(decodedBuf));
	int len = BleToolsUtil_base64Decode(src.c_str(), strlen(src.c_str()), decodedBuf);
	memcpy(dest, decodedBuf, destLength);
}

static bool extractU2FRequestFromChrome(unsigned char *headerBuf, unsigned char *chromeMessage, unsigned int dataLen)
{
	// Chromeエクステンションから受信したJSONメッセージをparse
	std::string json = std::string((char *)chromeMessage, dataLen);
	parseChromeJSONMessage(json, &chromeU2FMessage);

	// parse失敗時はエラー
	if (chromeU2FMessage.empty) {
		return false;
	}

	if (chromeU2FMessage.enroll) {
		// Web-safe base64エンコードされたChallenge、appIDをデコード
		decodeWebsafeB64EncodeString(chromeU2FMessage.challengeHash, registerRequest.nonce, sizeof(registerRequest.nonce));
		decodeWebsafeB64EncodeString(chromeU2FMessage.appIdHash, registerRequest.appId, sizeof(registerRequest.appId));
	}

	if (chromeU2FMessage.sign) {
		// Web-safe base64エンコードされたChallenge、appID、keyHandleをデコード
		decodeWebsafeB64EncodeString(chromeU2FMessage.challengeHash, authRequest.nonce, sizeof(authRequest.nonce));
		decodeWebsafeB64EncodeString(chromeU2FMessage.appIdHash, authRequest.appId, sizeof(authRequest.appId));
		decodeWebsafeB64EncodeString(chromeU2FMessage.keyHandle, authRequest.keyHandle, sizeof(authRequest.keyHandle));
	}

	return true;
}

static size_t prepareBleU2fRequest(unsigned char *requestBuf)
{
	size_t pos = 0;

	// リクエストデータを配列にセット
	if (chromeU2FMessage.enroll) {
		requestBuf[0] = 0x00;
		requestBuf[1] = U2F_INS_REGISTER;
		requestBuf[2] = 0x00;
		requestBuf[3] = 0x00;
		requestBuf[4] = 0x00;
		requestBuf[5] = 0x00;
		requestBuf[6] = U2F_NONCE_SIZE + U2F_APPID_SIZE;

		pos = 7;
		memcpy(requestBuf + pos, reinterpret_cast<char *>(&registerRequest), requestBuf[6]);
		pos += requestBuf[6];
		requestBuf[pos++] = 0x00;
		requestBuf[pos++] = 0x00;
	}

	if (chromeU2FMessage.sign) {
		requestBuf[0] = 0x00;
		requestBuf[1] = U2F_INS_AUTHENTICATE;
		requestBuf[2] = U2F_AUTH_ENFORCE;
		requestBuf[3] = 0x00;
		requestBuf[4] = 0x00;
		requestBuf[5] = 0x00;
		requestBuf[6] = U2F_NONCE_SIZE + U2F_APPID_SIZE + 1 + authRequest.keyHandleLen;

		pos = 7;
		memcpy(requestBuf + pos, reinterpret_cast<char *>(&authRequest), requestBuf[6]);
		pos += requestBuf[6];
		requestBuf[pos++] = 0x00;
		requestBuf[pos++] = 0x00;
	}

	return pos;
}

static bool sendBleU2fRequest(pBleDevice dev, unsigned char *requestBuf, size_t requestLen)
{
	// ログ出力
	BleToolsUtil_outputLog("sendBleU2fRequest: Message to BLE");
	BleToolsUtil_outputDumpLog(requestBuf, requestLen);

	unsigned char replyCmd;
	memset(reply, 0x00, sizeof(reply));
	ReturnValue retval = dev->CommandWrite(FIDO_BLE_CMD_MSG, requestBuf, requestLen,
		&replyCmd, reply, &replyLength);
	if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		BleToolsUtil_outputLog("sendBleU2fRequest: Command write failed");
		return false;
	}

	// 戻りのコマンドをチェック
	if (replyCmd != FIDO_BLE_CMD_MSG) {
		BleToolsUtil_outputLog("sendBleU2fRequest: reply command != FIDO_BLE_CMD_MSG");
		return false;
	}

	// 戻りのステータスワードをチェック
	chromeU2FMessage.statusWordNumber = bytes2short(reply, replyLength - 2);
	if (chromeU2FMessage.statusWordNumber != FIDO_RESP_SUCCESS) {
		BleToolsUtil_outputLog("sendBleU2fRequest: status word != FIDO_RESP_SUCCESS");
		return false;
	}

	// ログ出力
	BleToolsUtil_outputLog("sendBleU2fRequest: Message from BLE");
	BleToolsUtil_outputDumpLog(reply, replyLength);

	return true;
}

static void encodeWebsafeB64String(unsigned char *src, size_t srcLength)
{
	// B64エンコードを実行
	memset(encodedBuf, 0x00, sizeof(encodedBuf));
	int len = BleToolsUtil_base64Encode((char *)src, srcLength, encodedBuf);

	// ログ出力
	BleToolsUtil_outputLog("encodeWebsafeB64String");
	BleToolsUtil_outputLog((char *)encodedBuf);
}

static bool sendU2FRequestToBleDevice(pBleDevice dev)
{
	// BLE U2Fリクエストを生成
	size_t requestlen = prepareBleU2fRequest(request);

	// BLE U2Fリクエストを転送
	if (sendBleU2fRequest(dev, request, requestlen) == false) {
		return false;
	}

	// BLEからのレスポンスからステータスワードを除去し、Web Safe Base64エンコード
	reply[replyLength - 1] = 0x00;
	reply[replyLength - 2] = 0x00;
	encodeWebsafeB64String(reply, replyLength - 2);

	// レスポンスデータ編集完了
	return true;
}

static std::string serializeChromeJSONMessage(void)
{
	// レスポンスデータをJSONオブジェクトにシリアライズ
	picojson::object obj;
	if (chromeU2FMessage.enroll) {
		obj.insert(std::make_pair("type",       picojson::value("enroll_helper_reply")));
		obj.insert(std::make_pair("version",    picojson::value(chromeU2FMessage.version)));
		obj.insert(std::make_pair("code",       picojson::value(0.0)));
		obj.insert(std::make_pair("enrollData", picojson::value((char *)encodedBuf)));
	}

	// シリアライズされたデータを文字列化
	picojson::value val(obj);
	return val.serialize();
}

bool returnU2FResponseToChrome(void)
{
	// レスポンスデータをシリアライズし
	// JSON文字列を生成
	std::string jsonString = serializeChromeJSONMessage();
	const char *jsonStringBytes = jsonString.c_str();

	// ログ出力
	BleToolsUtil_outputLog("returnU2FResponseToChrome");
	BleToolsUtil_outputLog(jsonStringBytes);

	// ヘッダーを標準出力
	size_t jsonStringLen = jsonString.length();
	putchar(jsonStringLen % 256);
	putchar(jsonStringLen / 256);
	putchar(0x00);
	putchar(0x00);

	// JSON文字列を標準出力する
	std::cout << jsonString;
	return true;
}

// 正しくChromeサブプロセスを終了させるため、
// 処理失敗時はエコーバックを行う
void echoBack(unsigned char *headerBuf, unsigned char *dataBuf, unsigned int dataLen)
{
	for (size_t i = 0; i < 4; i++)
		putchar(headerBuf[i]);
	for (size_t i = 0; i < dataLen; i++)
		putchar(dataBuf[i]);
}

bool BleChromeHelper_ProcessNativeMessage(pBleDevice dev)
{
	// Chromeエクステンションと標準入出力によりやりとりを行う
	BleToolsUtil_outputLog("BleChromeHelper_ProcessNativeMessage start");
	unsigned char ch;
	unsigned char headerBuf[4];
	unsigned int  dataLen;
	unsigned char dataBuf[512];

	// カウンターをリセット
	unsigned int headerReadCnt = 0;
	unsigned int dataReadCnt = 0;

	// 標準入力から文字を読込
	while (std::cin >> ch) {
		// ヘッダーバイトを読込
		if (headerReadCnt < sizeof(headerBuf)) {
			headerBuf[headerReadCnt++] = ch;
			continue;
		}
		// ヘッダーバイトで指示された長さを保持
		if (dataReadCnt == 0) {
			dataLen = (unsigned int)headerBuf[1] * 256 + (unsigned int)headerBuf[0];
			memset(dataBuf, 0x00, sizeof(dataBuf));
		}
		// 標準入力からメッセージ本体を読込
		dataBuf[dataReadCnt++] = ch;
		if (dataReadCnt == dataLen) {
			// U2Fリクエストデータを取得
			if (extractU2FRequestFromChrome(headerBuf, dataBuf, dataLen) == false) {
				// 処理失敗時はエコーバック
				echoBack(headerBuf, dataBuf, dataLen);
				return false;
			}
			// BLEデバイスにリクエストを転送
			if (sendU2FRequestToBleDevice(dev) == false) {
				// 処理失敗時はエコーバック
				echoBack(headerBuf, dataBuf, dataLen);
				return false;
			}
			// U2FレスポンスデータをChromeに戻す
			if (returnU2FResponseToChrome() == false) {
				// 処理失敗時はエコーバック
				echoBack(headerBuf, dataBuf, dataLen);
				return false;
			}
			// カウンターをリセット
			headerReadCnt = 0;
			dataReadCnt = 0;
		}
	}

	BleToolsUtil_outputLog("BleChromeHelper_ProcessNativeMessage end");
	return true;
}
