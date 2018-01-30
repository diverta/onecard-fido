#include "u2f.h"

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
	std::string code;
	bool enroll;
	bool sign;
	bool empty;
} CHROME_U2F_MESSAGE;

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
	unsigned char decodedBuf[40];

	memset(decodedBuf, 0x00, sizeof(decodedBuf));
	int len = BleToolsUtil_base64Decode(src.c_str(), strlen(src.c_str()), decodedBuf);
	memcpy(dest, decodedBuf, destLength);
}

static bool extractU2FRequestFromChrome(unsigned char *headerBuf, unsigned char *chromeMessage, unsigned int dataLen)
{
	// Chromeエクステンションから受信したJSONメッセージをparse
	CHROME_U2F_MESSAGE u2fRequestMessage;
	std::string json = std::string((char *)chromeMessage, dataLen);
	parseChromeJSONMessage(json, &u2fRequestMessage);

	// parse失敗時はエラー
	if (u2fRequestMessage.empty) {
		return false;
	}

	if (u2fRequestMessage.enroll) {
		// Web-safe base64エンコードされたChallenge、appIDをデコード
		decodeWebsafeB64EncodeString(u2fRequestMessage.challengeHash, registerRequest.nonce, sizeof(registerRequest.nonce));
		decodeWebsafeB64EncodeString(u2fRequestMessage.appIdHash, registerRequest.appId, sizeof(registerRequest.appId));
	}

	if (u2fRequestMessage.sign) {
		// Web-safe base64エンコードされたChallenge、appID、keyHandleをデコード
		decodeWebsafeB64EncodeString(u2fRequestMessage.challengeHash, authRequest.nonce, sizeof(authRequest.nonce));
		decodeWebsafeB64EncodeString(u2fRequestMessage.appIdHash, authRequest.appId, sizeof(authRequest.appId));
		decodeWebsafeB64EncodeString(u2fRequestMessage.keyHandle, authRequest.keyHandle, sizeof(authRequest.keyHandle));
	}

	return true;
}

bool BleChromeHelper_ProcessNativeMessage(void)
{
	// Chromeエクステンションと標準入出力によりやりとりを行う
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
				return false;
			}

			//
			// Echo back (仮コード)
			for (size_t i = 0; i < 4; i++)
				putchar(headerBuf[i]);
			for (size_t i = 0; i < dataLen; i++)
				putchar(dataBuf[i]);
			//

			// カウンターをリセット
			headerReadCnt = 0;
			dataReadCnt = 0;
		}
	}

	return true;
}
