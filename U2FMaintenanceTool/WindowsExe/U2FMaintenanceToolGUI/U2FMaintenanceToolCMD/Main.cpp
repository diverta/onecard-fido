#include <string>
#include <iostream>

#include "BleApi.h"
#include "ble_util.h"
#include "BleApiWinRT.h"

#include "BleTools.h"
#include "BleToolsUtil.h"

int   arg_Verbose = 0;
char *arg_DeviceIdentifier = NULL;

BleApiConfiguration  configuration;
pBleDevice           dev = NULL;

static void promptPairing(void)
{
	if (arg_pairing == true) {
		// ペアリング実行時のメッセージを表示
		std::cout << "One Cardとのペアリングを実行します. " << std::endl;
		std::cout << "  One Cardをペアリングモードに変更してください." << std::endl;
		std::cout << "  One CardのMAIN SWを５秒以上押し続けると、ペアリングモードに変更できます." << std::endl;
		std::cout << "30秒ほど待機します. しばらくお待ちください..." << std::endl;
	} else {
		// ペアリング実行時でない場合は、ペアリングを要求する旨のメッセージを表示
		std::cout << "One Cardが見つかりません. " << std::endl;
		std::cout << "まず最初に、One Cardとのペアリングを実行してください." << std::endl;
	}
}

static pBleDevice selectPairedBLEDevice(pBleApi api)
{
	// U2Fデバイスを探索
	std::vector<pBleDevice> devices = api->findDevices();
	if (devices.size() == 0) {
		return nullptr;
	}

	// デバイスが検索できた場合はペアリング済のメッセージを表示
	if (arg_pairing == true) {
		std::cout << "既にOne Cardとペアリング済みです. " << std::endl;
	}

	if (arg_DeviceIdentifier) {
		// パラメーターで指定されたU2Fデバイスを選択
		std::string id(arg_DeviceIdentifier);
		std::vector<pBleDevice>::iterator i;
		for (i = devices.begin(); i != devices.end(); i++) {
			if (((*i)->Identifier() == id)
				&& ((*i)->Identifier().length() == id.length()))
				return (*i);
		}

	} else {
		// 指定されていない場合は、先頭のU2Fデバイスを選択
		return devices[0];
	}

	return nullptr;
}

static pBleDevice pairingBLEDevice(pBleApi api)
{
	// プロンプトを表示
	promptPairing();

	if (arg_pairing == false) {
		BleToolsUtil_outputLog("prepareBLEDevice: BLE device not found");
		return NULL;
	}

	// ペアリングされていないデバイスとペアリングを実行
	BleApiWinRT *apiWinRT = (BleApiWinRT *)api;
	return apiWinRT->bondWithUnpairedDevice();
}

static int prepareBLEDevice(BleApiConfiguration &configuration)
{
	// BLE通信が不要の場合は終了
	if (arg_need_ble == false) {
		return 0;
	}

	// BLE APIの準備
	pBleApi api = BleApi::CreateAPI(configuration);
	if (!api->IsEnabled()) {
		BleToolsUtil_outputLog("prepareBLEDevice: BLE API is disabled");
		return -1;
	}

	// ペアリング済みU2Fデバイスを探索
	dev = selectPairedBLEDevice(api);

	// ペアリング済みU2Fデバイスがない場合はペアリング実行
	if (dev == nullptr) {
		dev = pairingBLEDevice(api);
	}

	// ペアリング済みU2Fデバイスがない場合は以降の処理を行わない
	if (dev == nullptr) {
		std::cout << "使用できるFIDO BLE U2Fデバイスがありません." << std::endl;
		BleToolsUtil_outputLog("prepareBLEDevice: No BLE device available");
		return -1;
	}

	// 選択されたデバイスを検証
	if (dev->Verify() != ReturnValue::BLEAPI_ERROR_SUCCESS) {
		return -1;
	}

	// デバイスのタイムアウトを30秒に設定
	dev->SetTimeout(30000);
	BleToolsUtil_outputLog("prepareBLEDevice success");
	return 0;
}

static void flushAndClose(void)
{
	std::cout.flush();
	std::cerr.flush();
	std::wcout.flush();
	std::wcerr.flush();
}

int __main(int argc, char *argv[])
{
	try {
		// コマンドライン引数を取得
		if (BleTools_ParseArguments(argc, argv, configuration) != 0) {
			return -1;
		}

		// BLEデバイスを選択
		if (prepareBLEDevice(configuration) != 0) {
			return -1;
		}

		// 引数なしで起動された場合は
		// 画面にデバイス名を表示
		if (argc == 1) {
			std::cout << "FIDO BLE U2F Maintenance Tool " << std::endl << std::endl;
			std::cout << "==== 選択されたFIDO BLE U2Fデバイス ====" << std::endl;
			dev->Report();
		}

		// コマンドラインで指定されたコマンドを実行
		if (BleTools_ProcessCommand(configuration, dev) != 0) {
			return -1;
		}

	} catch (std::exception e) {
		BleToolsUtil_outputLog("main: Exception raised");
		BleToolsUtil_outputLog(e.what());
		std::cout << "ERROR: " << e.what() << std::endl;
		std::cout << std::endl << "ツールの実行が失敗しました." << std::endl;
		return -1;
	}

	flushAndClose();
	return 0;
}

int __cdecl main(Platform::Array<Platform::String^>^ args)
{
	// 引数の数を取得
	int argc = args->Length;

	// 引数格納する領域を確保
	char **argv = (char **)malloc(sizeof(char *) * argc);
	if (argv == nullptr) {
		std::cout << "コマンド引数の格納領域確保に失敗しました." << std::endl;
		return -1;
	}

	int i = 0;
	for (Platform::String^ arg : args) {
		// アドレスポインターwchar型をchar型に変換
		std::wstring wstr(arg->Begin());
		std::string  cstr(wstr.begin(), wstr.end());
		const char  *char_str = cstr.c_str();

		// char型配列を別領域に格納
		size_t slen = strlen(char_str);
		char *sp = (char *)malloc(slen + 1);
		memcpy_s(sp, slen, char_str, slen);

		// 格納先のアドレスポインターを保持
		argv[i++] = sp;
	}

	// メイン処理を実行
	int ret = __main(argc, argv);

	// 確保した引数格納領域を解放
	for (i = 0; i < argc; i++) {
		free(argv[i]);
	}
	free(argv);

	return ret;
}
