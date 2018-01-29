#include <string>
#include <iostream>
#include <fstream>

#include "fido_ble.h"
#include "fido_apduresponses.h"
#include "ble_util.h"

#include "BleTools.h"

//
// 実行させるコマンド／引数を保持
//
bool  arg_erase_bonding     = false;
bool  arg_erase_skey_cert   = false;
bool  arg_install_skey_cert = false;
char *arg_skey_file_path    = NULL;
char *arg_cert_file_path    = NULL;
bool  arg_chrome_nm_setup   = false;

//
// U2Fサービスからの返信データを受領するための領域とフラグ
//
#define REPLY_BUFFER_LENGTH 512
static unsigned char fragmentReplyBuffer[REPLY_BUFFER_LENGTH];
static unsigned int  fragmentReplyBufferLength;
static bool          eventDone = true;

static void BleTools_TransportEventHandler(BleDevice::FIDOEventType type, unsigned char *buffer, unsigned int bufferLength)
{
	if (eventDone) {
		// フラグが受領済みの場合は無視
		return;
	}

	if (buffer[0] == FIDO_BLE_CMD_KEEPALIVE) {
		// キープアライブは無視
		return;
	}

	// BLE U2Fサービスから受信したデータをバッファに設定し、
	// フラグを受領済みに設定
	memcpy(fragmentReplyBuffer, buffer, bufferLength);
	fragmentReplyBufferLength = bufferLength;
	eventDone = true;
}

static void BleToolsEventHandler(BleDevice::FIDOEventType type, unsigned char *buffer, unsigned int bufferLength)
{
	if (type == BleDevice::EVENT_FRAGMENT) {
		// BLE U2Fからデータを受信した時の処理を実行
		BleTools_TransportEventHandler(type, buffer, bufferLength);
	}
}

static bool doCommandWrite(pBleDevice dev, unsigned char *request, unsigned int requestLength)
{
	ReturnValue   retval;
	unsigned char reply[16];
	unsigned int  replyLength = sizeof(reply);
	unsigned char replyCmd;

	// コマンド（FIDO_BLE_CMD_MSG）を実行
	retval = dev->CommandWrite(FIDO_BLE_CMD_MSG, request, requestLength, &replyCmd, reply, &replyLength);

	if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		// BLE APIの戻りステータスが不正の場合はエラー
		std::cerr << "[doCommandWrite] CommandWrite failed" << std::endl;
		return false;
	}

	if (replyCmd != FIDO_BLE_CMD_MSG) {
		// U2Fサービスの戻りコマンドが不正の場合はエラー
		std::cerr << "[doCommandWrite] Reply is not a FIDO_BLE_CMD_MSG (0x83)" << std::endl;
		return false;
	}

	if (bytes2short(reply, replyLength - 2) != FIDO_RESP_SUCCESS) {
		// U2Fサービスの戻りコマンドが不正の場合はエラー
		std::cerr << "[doCommandWrite] Status code is not FIDO_RESP_SUCCESS (0x9000)" << std::endl;
		return false;
	}

	return true;
}

static bool processEraseBonding(BleApiConfiguration &configuration, pBleDevice dev)
{
	unsigned char request[4];

	// リクエストデータ（APDU）を編集し request に格納
	// INS=0x40, P1=0x01
	request[0] = 0x00;
	request[1] = 0x40;
	request[2] = 0x01;
	request[3] = 0x00;

	// コマンド（FIDO_BLE_CMD_MSG）を実行
	if (doCommandWrite(dev, request, sizeof(request)) == false) {
		return false;
	}

	std::cout << "ペアリング情報をFlash ROMから削除しました。" << std::endl;
	return true;
}

static bool processEraseSkeyCert(BleApiConfiguration &configuration, pBleDevice dev)
{
	unsigned char request[4];

	// リクエストデータ（APDU）を編集し request に格納
	// INS=0x40, P1=0x02
	request[0] = 0x00;
	request[1] = 0x40;
	request[2] = 0x02;
	request[3] = 0x00;

	// コマンド（FIDO_BLE_CMD_MSG）を実行
	if (doCommandWrite(dev, request, sizeof(request)) == false) {
		return false;
	}

	std::cout << "鍵・証明書をFlash ROMから削除しました。" << std::endl;
	return true;
}

static int readFile(char *file_path, char *buffer)
{
	int readSize = 0;
	try {
		// ファイルを読込む
		std::ifstream ifs(file_path, std::ios::binary);
		if (ifs.fail()) {
			return -1;
		}

		// ファイルサイズを取得する
		ifs.seekg(0, ifs.beg);
		int begin = static_cast<int>(ifs.tellg());
		ifs.seekg(0, ifs.end);
		int end   = static_cast<int>(ifs.tellg());
		readSize  = end - begin;

		// ファイルの内容を先頭から読込んで
		// 引数の領域に格納する
		ifs.clear();
		ifs.seekg(0, ifs.beg);
		ifs.read(buffer, readSize);
		ifs.close();

	} catch (std::exception e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		std::cerr << "ファイル[" << file_path << "]の読込が失敗しました." << std::endl;
		return -1;
	}

	return readSize;
}

static inline int convertBase64CharTo6bitValue(int c)
{
	// base64の1文字を6bitの値に変換する
	if (c == '=')
		return 0;
	if (c == '/')
		return 63;
	if (c == '+')
		return 62;
	if (c <= '9')
		return (c - '0') + 52;
	if ('a' <= c)
		return (c - 'a') + 26;
	return (c - 'A');
}

static int base64Decode(const char* src, unsigned char* dest) 
{
	// base64の文字列srcをデコードしてdestに格納
	unsigned char  o0, o1, o2, o3;
	unsigned char *p = dest;
	for (int n = 0; src[n];) {
		o0 = convertBase64CharTo6bitValue(src[n]);
		o1 = convertBase64CharTo6bitValue(src[n + 1]);
		o2 = convertBase64CharTo6bitValue(src[n + 2]);
		o3 = convertBase64CharTo6bitValue(src[n + 3]);

		*p++ = (o0 << 2) | ((o1 & 0x30) >> 4);
		*p++ = ((o1 & 0xf) << 4) | ((o2 & 0x3c) >> 2);
		*p++ = ((o2 & 0x3) << 6) | o3 & 0x3f;
		n += 4;
	}
	*p = 0;

	// 変換後のバイト数を返す
	return int(p - dest);
}

static bool readPemFile(char *file_path, unsigned char *key_buffer)
{
	// PEMファイルデータ格納領域を確保
	unsigned char *pem_buffer = (unsigned char *)malloc(256);
	if (pem_buffer == NULL) {
		return false;
	}

	try {
		// ファイルを読込む
		std::ifstream ifs(file_path);
		if (ifs.fail()) {
			free(pem_buffer);
			return false;
		}

		std::string strTmp;
		std::string strPem;
		const char *p;
		while (getline(ifs, strTmp)) {
			// ヘッダー／フッターは読み飛ばす
			p = strTmp.c_str();
			if (strcmp("-----BEGIN EC PRIVATE KEY-----", p) == 0) {
				continue;
			}
			if (strcmp("-----END EC PRIVATE KEY-----", p) == 0) {
				continue;
			}
			// 一時バッファに連結
			strPem += strTmp;
		}
		ifs.close();

		// 秘密鍵はPEMファイルの先頭8バイト目から32バイトなので、
		// 先頭からリトルエンディアン形式で配置しなおす。
		int len = base64Decode(strPem.c_str(), pem_buffer);
		for (int i = 0; i < 32; i++) {
			key_buffer[31 - i] = pem_buffer[7 + i];
		}

		free(pem_buffer);
		return true;

	} catch (std::exception e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		std::cerr << "ファイル[" << file_path << "]の読込が失敗しました." << std::endl;
		free(pem_buffer);
		return false;
	}
}

// 秘密鍵読込用エリア
static unsigned char skeyBuffer[32];
static bool processInstallSkey(BleApiConfiguration &configuration, pBleDevice dev)
{
	// 秘密鍵ファイルを読込む
	//   skey_bufferの先頭から32バイト分、
	//   リトルエンディアン形式で格納される
	if (readPemFile(arg_skey_file_path, skeyBuffer) == false) {
		return false;
	}

	// リクエストデータ格納領域を確保
	// データ長＝ヘッダー(7)＋鍵の長さ(32)＋Le(2)
	size_t requestLength = 41;
	unsigned char *request = (unsigned char *)malloc(requestLength);
	if (request == NULL) {
		return false;
	}

	// リクエストデータ（APDU）を編集し request に格納
	//   INS=0x40, P1=0x03
	request[0] = 0x00;
	request[1] = 0x40;
	request[2] = 0x03;
	request[3] = 0x00;
	request[4] = 0x00;
	//   データ長さ(32バイト)を設定
	request[5] = 0;
	request[6] = 32;
	//   データを設定
	memcpy(request + 7, skeyBuffer, 32);
	//   Leを設定
	request[requestLength - 2] = 0x00;
	request[requestLength - 1] = 0x00;

	// コマンド（FIDO_BLE_CMD_MSG）を実行
	bool ret = doCommandWrite(dev, request, requestLength);
	if (ret == true) {
		std::cout << "秘密鍵をFlash ROMにインストールしました。" << std::endl;
	}

	free(request);
	return ret;
}

// 証明書読込用エリア
static char certBuffer[1024];
static bool processInstallCert(BleApiConfiguration &configuration, pBleDevice dev)
{
	// 証明書ファイルを読込む
	int certBufferSize = readFile(arg_cert_file_path, certBuffer);
	if (certBufferSize < 1) {
		return false;
	}

	// リクエストデータ格納領域を確保
	unsigned char *request = (unsigned char *)malloc(certBufferSize + 9);
	if (request == NULL) {
		return false;
	}

	// リクエストデータ（APDU）を編集し request に格納
	//   INS=0x40, P1=0x04
	request[0] = 0x00;
	request[1] = 0x40;
	request[2] = 0x04;
	request[3] = 0x00;
	request[4] = 0x00;
	//   データ長さを設定
	request[5] = certBufferSize / 256;
	request[6] = certBufferSize % 256;
	//   データを設定
	memcpy(request + 7, certBuffer, certBufferSize);
	//   Leを設定
	request[7 + certBufferSize + 0] = 0x00;
	request[7 + certBufferSize + 1] = 0x00;

	// コマンド（FIDO_BLE_CMD_MSG）を実行
	bool ret = doCommandWrite(dev, request, certBufferSize + 9);
	if (ret == true) {
		std::cout << "証明書をFlash ROMにインストールしました。" << std::endl;
	}

	free(request);
	return ret;
}

static bool processInstallSkeyCert(BleApiConfiguration &configuration, pBleDevice dev)
{
	std::cout << "以下の鍵・証明書がインストールされます。" << std::endl;
	std::cout << "鍵ファイルパス    : " << arg_skey_file_path << std::endl;
	std::cout << "証明書ファイルパス: " << arg_cert_file_path << std::endl;
	std::cout << std::endl;

	// 鍵ファイルをインストール
	if (processInstallSkey(configuration, dev) == false) {
		return false;
	}

	// 証明書ファイルをインストール
	if (processInstallCert(configuration, dev) == false) {
		return false;
	}

	return true;
}

static bool getExecutableDirectory(char *executableFilePath, int executableFilePathMaxLen)
{
	// 実行可能ファイルの絶対パスを取得
	if (GetModuleFileName(NULL, executableFilePath, executableFilePathMaxLen) == 0) {
		// 取得ができないときはエラー
		std::cout << "processChromeNativeMessagingSetup: GetModuleFileName failed" << std::endl;
		return false;
	}

	// 実行可能ファイルが配置されているディレクトリーを取得
	int endPos;
	int size_ = strnlen(executableFilePath, executableFilePathMaxLen);
	for (int i = 0; i < size_; i++) {
		endPos = size_ - i - 1;
		if (executableFilePath[endPos] == 0x5c) {
			// \ マーク（0x5c）が見つかったら
			// 終端文字（0x00）に変えてファイル名部分を切り落とす
			executableFilePath[endPos] = 0x00;
			break;
		}
	}

	if (endPos == 0) {
		// ディレクトリーが取得できない場合はエラー
		std::cout << "processChromeNativeMessagingSetup: Executable directory get failed" << std::endl;
		return false;
	}

	return true;
}

static bool createRegistryEntry(char *jsonFileFullPath)
{
	const char *registryKey = "Software\\Google\\Chrome\\NativeMessagingHosts\\jp.co.diverta.chrome.helper.ble.u2f";

	std::cout << "以下の項目がレジストリーに登録されます。" << std::endl;
	std::cout << "レジストリーキー: " << registryKey        << std::endl;
	std::cout << "JSONファイルパス: " << jsonFileFullPath   << std::endl;
	std::cout << std::endl;

	HKEY hKey;
	DWORD dwDisposition;
	if (RegCreateKeyEx(HKEY_CURRENT_USER,
		registryKey,
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		&dwDisposition) != ERROR_SUCCESS) {
		// レジストリーキーが生成できない場合はエラー
		std::cout << "processChromeNativeMessagingSetup: Registry key create failed" << std::endl;
		return false;
	}

	if (RegSetValueEx(
		hKey,
		"",
		0,
		REG_SZ,
		(CONST BYTE*)(LPCTSTR)jsonFileFullPath,
		(int)strlen(jsonFileFullPath)
	)) {
		// レジストリーキーに値がセットできない場合はエラー
		std::cout << "processChromeNativeMessagingSetup: Registry value set failed" << std::endl;
		return false;
	}

	return true;
}

static bool processChromeNativeMessagingSetup(void)
{
	// Chrome Native Messagingを有効化するため
	// 設定用JSONファイルパスをレジストリーに登録
	const char *jsonFileName = "jp.co.diverta.chrome.helper.ble.u2f.json";

	// 実行可能ファイルの絶対パスを取得
	char executableFilePath[256];
	if (getExecutableDirectory(executableFilePath, sizeof(executableFilePath) - 1) == false) {
		// 取得ができないときはエラー
		return false;
	}

	// JSONファイルパスを編集
	char jsonFileFullPath[255];
	sprintf_s(jsonFileFullPath, "%s\\%s", executableFilePath, jsonFileName);

	// 設定用JSONファイルパスをレジストリーに登録
	if (createRegistryEntry(jsonFileFullPath) == false) {
		// 登録ができないときはエラー
		return false;
	}

	return true;
}

int BleTools_ProcessCommand(BleApiConfiguration &configuration, pBleDevice dev)
{
	if (!dev->NotificationsRegistered()) {
		// BLE U2Fからデータ受信時の処理を登録し、
		// 受信通知を有効化
		ReturnValue retval = dev->RegisterNotifications(BleToolsEventHandler);
		if (retval != ReturnValue::BLEAPI_ERROR_SUCCESS) {
			throw std::runtime_error(__FILE__ ":" + std::to_string(__LINE__) + ": could not register notification although we are connected.");
		}
	}

	if (arg_erase_bonding) {
		// ペアリング情報をFlash ROMから削除
		if (processEraseBonding(configuration, dev) == false) {
			return -1;
		}
	}

	if (arg_erase_skey_cert) {
		// 鍵・証明書をFlash ROMから削除し、
		// AES鍵を自動生成
		if (processEraseSkeyCert(configuration, dev) == false) {
			return -1;
		}
	}

	if (arg_install_skey_cert) {
		// 鍵・証明書をインストール
		if (processInstallSkeyCert(configuration, dev) == false) {
			return -1;
		}
	}

	if (arg_chrome_nm_setup) {
		// Chrome Native Messaging有効化設定
		if (processChromeNativeMessagingSetup() == false) {
			return -1;
		}
	}

	return 0;
}

static bool pathFileExists(char *file_path)
{
	std::ifstream ifs(file_path);
	if (ifs.is_open() == false) {
		return false;
	}
	ifs.close();
	return true;
}

static bool checkSkeyCertPath(char *skey_file_path, char *cert_file_path)
{
	if (pathFileExists(skey_file_path) == false) {
		std::cerr << "正しい鍵ファイルパスを指定してください。" << std::endl;
		return false;
	}
	if (pathFileExists(cert_file_path) == false) {
		std::cerr << "正しい証明書ファイルパスを指定してください。" << std::endl;
		return false;
	}
	return true;
}

void BleTools_ChromeNativeMessageReceived(unsigned char *headerBuf, unsigned char *chromeMessage, unsigned int dataLen)
{
	// Echo back
	for (size_t i = 0; i < 4; i++)
		putchar(headerBuf[i]);

	for (size_t i = 0; i < dataLen; i++)
		putchar(chromeMessage[i]);
}

void BleTools_ProcessChromeNativeMessage(void)
{
	// Chromeエクステンションと標準入出力によりやりとりを行う
	unsigned char ch;
	unsigned char headerBuf[4];
	unsigned int  dataLen;
	unsigned char dataBuf[512];

	// カウンターをリセット
	unsigned int headerReadCnt = 0;
	unsigned int dataReadCnt   = 0;

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
			// データを処理
			BleTools_ChromeNativeMessageReceived(headerBuf, dataBuf, dataLen);
			// カウンターをリセット
			headerReadCnt = 0;
			dataReadCnt = 0;
		}
	}
}

int BleTools_ParseArguments(int argc, char *argv[], BleApiConfiguration &configuration)
{
	int count = 1;
	while (count < argc) {
		if (!strncmp(argv[count], "-v", 2)) {
			configuration.logging |= BleApiLogging::Info;
			arg_Verbose |= 1;
		}
		if (!strncmp(argv[count], "-V", 2)) {
			configuration.logging |= BleApiLogging::Debug;
			arg_Verbose |= 2;
		}
		if (!strncmp(argv[count], "-a", 2)) {
			arg_Abort = false;
		}
		if (!strncmp(argv[count], "-p", 2)) {
			arg_Pause = true;
		}
		if (!strncmp(argv[count], "-w", 2)) {
			arg_LethalWarn = false;
		}
		if (!strncmp(argv[count], "-T", 2)) {
			configuration.logging |= BleApiLogging::Tracing;
		}
		if (!strncmp(argv[count], "-d", 2)) {
			++count;
			if (count == argc) {
				std::cerr << "-d の後に対象のBLEデバイス名を指定してください。" << std::endl;
				return -1;
			}
			arg_DeviceIdentifier = argv[count];
		}
		if (!strncmp(argv[count], "-C", 2)) {
			configuration.continuous = true;
		}
		if (!strncmp(argv[count], "-D", 2)) {
			configuration.alwaysconnected = true;
		}
		if (!strncmp(argv[count], "-B", 2)) {
			// ペアリング情報をFlash ROMから削除
			arg_erase_bonding = true;
		}
		if (!strncmp(argv[count], "-E", 2)) {
			// 鍵・証明書をFlash ROMから削除し、
			// AES鍵を自動生成する
			arg_erase_skey_cert = true;
		}
		if (!strncmp(argv[count], "-I", 2)) {
			if (++count == argc) {
				// あとに引数が続かない場合はエラー
				std::cerr << "-I の後に[鍵ファイルパス]を指定してください。" << std::endl;
				return -1;
			}
			// 鍵ファイル名
			arg_skey_file_path = argv[count];
			if (++count == argc) {
				// あとに引数が続かない場合はエラー
				std::cerr << "-I [鍵ファイルパス]の後に[証明書ファイルパス]を指定してください。" << std::endl;
				return -1;
			}
			// 証明書ファイル名
			arg_cert_file_path = argv[count];
			if (checkSkeyCertPath(arg_skey_file_path, arg_cert_file_path) == false) {
				// ファイルパスのチェック結果がNGの場合はエラー
				return -1;
			}
			// 鍵・証明書ファイルをインストール
			arg_install_skey_cert = true;
		}
		if (!strncmp(argv[count], "-R", 2)) {
			// Chrome Native Messaging有効化設定
			arg_chrome_nm_setup = true;
		}
		if (!strncmp(argv[count], "chrome-extension://", 19)) {
			// Chromeのサブプロセスとして起動
			BleTools_ProcessChromeNativeMessage();
			return 0;
		}
		++count;
	}

	return 0;
}
