#include <string>
#include <iostream>

#include "BleApi.h"
#include "ble_util.h"

#include "BleTools.h"

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
	std::cout << "FIDO BLE U2F Maintenance Tool " << std::endl << std::endl;

	// BLE APIの準備
	pBleApi api = BleApi::CreateAPI(configuration);
	if (!api->IsEnabled()) {
		return -1;
	}

	// U2Fデバイスを探索
	std::vector<pBleDevice> devices = api->findDevices();
	if (!devices.size()) {
		// デバイスが検索できない場合はペアリングを要求
		promptPairing();
		return -1;
	}

	if (devices.size() > 1) {
		// 探索されたU2Fデバイスが複数あったばあいは
		// 有効なデバイスを一覧表示
		std::vector<pBleDevice>::iterator i;
		std::cout << "使用できるFIDO BLE U2Fデバイス:" << std::endl;
		for (i = devices.begin(); i != devices.end(); i++) {
			std::cout << "  " << (*i)->Identifier() << std::endl;
		}
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
		return -1;
	}

	// デバイスのタイムアウトを30秒に設定
	dev->SetTimeout(30000);
	return 0;
}

static void flushAndClose(void)
{
	std::cout.flush();
	std::cerr.flush();
	std::wcout.flush();
	std::wcerr.flush();
}

static void displayDeviceSpecs(void)
{
	std::cout << "==== 選択されたFIDO BLE U2Fデバイス ====" << std::endl;
	dev->Report();
	dev->Verify();
	std::cout << std::endl;
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

		// 選択されたデバイスの詳細情報を表示
		displayDeviceSpecs();

		// コマンドラインで指定されたコマンドを実行
		if (BleTools_ProcessCommand(configuration, dev) != 0) {
			return -1;
		}

	} catch (std::exception e) {
		std::cout << "ERROR: " << e.what() << std::endl;
		std::cout << std::endl << "ツールの実行が失敗しました." << std::endl;
		return -1;
	}

	flushAndClose();
	return 0;
}
