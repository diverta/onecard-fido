#include "ble_util.h"
#include "BleApi.h"
#include "fido_ble.h"
#include "fido_apduresponses.h"

#include "BleToolsU2F.h"
#include "BleToolsUtil.h"

// For JSON parse/serialize
#include "picojson.h"

// appId、challenge、キーハンドルを保持
static unsigned char appIdBuf[U2F_APPID_SIZE];
static unsigned char challengeBuf[U2F_NONCE_SIZE];
static unsigned char keyHandleBuf[U2F_KEYHANDLE_SIZE];

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
		decodeWebsafeB64EncodeString(chromeU2FMessage.challengeHash, challengeBuf, sizeof(challengeBuf));
		decodeWebsafeB64EncodeString(chromeU2FMessage.appIdHash, appIdBuf, sizeof(appIdBuf));
	}

	if (chromeU2FMessage.sign) {
		// Web-safe base64エンコードされたChallenge、appID、keyHandleをデコード
		decodeWebsafeB64EncodeString(chromeU2FMessage.challengeHash, challengeBuf, sizeof(challengeBuf));
		decodeWebsafeB64EncodeString(chromeU2FMessage.appIdHash, appIdBuf, sizeof(appIdBuf));
		decodeWebsafeB64EncodeString(chromeU2FMessage.keyHandle, keyHandleBuf, sizeof(keyHandleBuf));
	}

	return true;
}

static bool sendU2FRequestToBleDevice(pBleDevice dev)
{
	// U2F Registerリクエストを転送
	if (chromeU2FMessage.enroll) {
		if (sendU2FRegisterRequest(dev, challengeBuf, appIdBuf) == false) {
			return false;
		}
	}

	// U2F Authenticateリクエストを転送
	if (chromeU2FMessage.sign) {
		if (sendU2FAuthenticateRequest(dev, challengeBuf, appIdBuf, keyHandleBuf) == false) {
			return false;
		}
	}

	// BLEレスポンスのステータスワードを取得
	chromeU2FMessage.statusWordNumber = BleToolsU2F_replyStatusWord();

	// BLEからのレスポンスからステータスワードを除去し、Web Safe Base64エンコード
	BleToolsU2F_encodeB64Reply(encodedBuf, sizeof(encodedBuf));

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
	if (chromeU2FMessage.sign) {
		picojson::object objSub;
		objSub.insert(std::make_pair("version",       picojson::value(chromeU2FMessage.version)));
		objSub.insert(std::make_pair("appIdHash",     picojson::value(chromeU2FMessage.appIdHash)));
		objSub.insert(std::make_pair("challengeHash", picojson::value(chromeU2FMessage.challengeHash)));
		objSub.insert(std::make_pair("keyHandle",     picojson::value(chromeU2FMessage.keyHandle)));
		objSub.insert(std::make_pair("signatureData", picojson::value((char *)encodedBuf)));

		obj.insert(std::make_pair("type",         picojson::value("sign_helper_reply")));
		obj.insert(std::make_pair("code",         picojson::value(0.0)));
		obj.insert(std::make_pair("responseData", picojson::value(objSub)));
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
	BleToolsUtil_outputLog("returnU2FResponseToChrome: response native message");
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
			// 正常終了時
			break;
		}
	}

	BleToolsUtil_outputLog("BleChromeHelper_ProcessNativeMessage end");
	return true;
}
