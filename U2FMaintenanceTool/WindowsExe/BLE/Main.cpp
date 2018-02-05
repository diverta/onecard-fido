#include <string>
#include <iostream>

#include "BleApi.h"
#include "ble_util.h"

#include "BleTools.h"
#include "BleToolsUtil.h"

int   arg_Verbose = 0;
char *arg_DeviceIdentifier = NULL;

BleApiConfiguration  configuration;
pBleDevice           dev = NULL;

static void promptPairing(void)
{
	std::cout << "One Cardが見つかりません. " << std::endl;
	std::cout << "まず最初に、One Cardをペアリングモードに変更し、ペアリングを実施してください." << std::endl;
	std::cout << "  One CardのMAIN SWを５秒以上押し続けると、ペアリングモードに変更できます." << std::endl;
}

static int prepareBLEDevice(BleApiConfiguration &configuration)
{
	// BLE APIの準備
	pBleApi api = BleApi::CreateAPI(configuration);
	if (!api->IsEnabled()) {
		BleToolsUtil_outputLog("prepareBLEDevice: BLE API is disabled");
		return -1;
	}

	// U2Fデバイスを探索
	std::vector<pBleDevice> devices = api->findDevices();
	if (!devices.size()) {
		// デバイスが検索できない場合はペアリングを要求
		promptPairing();
		BleToolsUtil_outputLog("prepareBLEDevice: BLE device not found");
		return -1;
	}

	if (arg_DeviceIdentifier) {
		// パラメーターで指定されたU2Fデバイスを選択
		std::string id(arg_DeviceIdentifier);
		std::vector<pBleDevice>::iterator i;
		for (i = devices.begin(); i != devices.end(); i++) {
			if (((*i)->Identifier() == id)
				&& ((*i)->Identifier().length() == id.length()))
				dev = (*i);
		}

	} else {
		// 指定されていない場合は、先頭のU2Fデバイスを選択
		dev = devices[0];
	}

	if (!dev) {
		std::cout << "使用できるFIDO BLE U2Fデバイスがありません." << std::endl;
		BleToolsUtil_outputLog("prepareBLEDevice: No BLE device available");
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

int __cdecl main(int argc, char *argv[])
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
