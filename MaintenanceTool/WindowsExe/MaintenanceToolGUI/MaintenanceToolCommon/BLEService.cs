﻿using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Devices.Radios;
using Windows.Storage.Streams;

namespace MaintenanceToolCommon
{
    public class BLEService
    {
        private GattDeviceService BLEservice;
        private GattCharacteristic U2FControlPointChar;
        private GattCharacteristic U2FStatusChar;

        private Guid U2F_BLE_SERVICE_UUID = new Guid("0000FFFD-0000-1000-8000-00805f9b34fb");
        private Guid U2F_CONTROL_POINT_CHAR_UUID = new Guid("F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB");
        private Guid U2F_STATUS_CHAR_UUID = new Guid("F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB");

        private BluetoothLEAdvertisementWatcher watcher;
        private ulong BluetoothAddress;

        public delegate void DataReceivedEvent(byte[] receivedData);
        public event DataReceivedEvent OnDataReceived;

        public delegate void TransactionFailedEvent();
        public event TransactionFailedEvent OnTransactionFailed;

        public delegate void FIDOPeripheralPairedEvent(bool success, string messageOnFail);
        public event FIDOPeripheralPairedEvent FIDOPeripheralPaired;

        public delegate void FIDOPeripheralFoundEvent();
        public event FIDOPeripheralFoundEvent FIDOPeripheralFound;

        // 例外が発生したかどうかを保持
        private bool critical;

        // サービスをディスカバーできたデバイスを保持
        private List<GattDeviceService> BLEServices = new List<GattDeviceService>();

        // ペアリングに使用するパスキー（PIN）を保持
        private string Passkey = null;

        public BLEService()
        {
            watcher = new BluetoothLEAdvertisementWatcher();
            watcher.Received += OnAdvertisementReceived;
            FreeResources();
        }

        public void Disconnect()
        {
            // 切断
            StopCommunicate();
            AppCommon.OutputLogInfo(AppCommon.MSG_U2F_DEVICE_DISCONNECTED);
        }

        public async void Send(byte[] requestData)
        {
            if (BLEservice == null) {
                AppCommon.OutputLogError(string.Format("BLEService.Send: service is null"));
                critical = true;
                OnTransactionFailed();
            }

            critical = false;
            try {
                // リクエストデータを生成
                DataWriter writer = new DataWriter();
                for (int i = 0; i < requestData.Length; i++) {
                    writer.WriteByte(requestData[i]);
                }

                // リクエストを実行（U2F Control Pointに書込）
                GattCommunicationStatus result = await U2FControlPointChar.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse);
                if (result != GattCommunicationStatus.Success) {
                    AppCommon.OutputLogError(AppCommon.MSG_REQUEST_SEND_FAILED);
                    OnTransactionFailed();
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.Send: {0}", e.Message));
                OnTransactionFailed();
            }
        }

        public async void Pair(string passkey)
        {
            // ペアリングに使用するパスキー（PIN）を保持
            Passkey = passkey;

            // Bluetoothがオンになっていることを確認
            bool bton = false;
            try {
                var radios = await Radio.GetRadiosAsync();
                foreach (var radio in radios) {
                    if (radio.Kind == RadioKind.Bluetooth) {
                        if (radio.State == RadioState.On) {
                            AppCommon.OutputLogInfo("Bluetoothはオンです。");
                            bton = true;
                            break;
                        }
                    }
                }
            } catch {
                // Bluetoothオン状態が確認できない場合はオフ状態であるとみなす
                AppCommon.OutputLogError("Bluetooth状態を確認できません。");
            }

            if (!bton) {
                // 画面スレッドに失敗を通知
                FIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_BT_OFF);
                AppCommon.OutputLogError("Bluetoothはオフです。");
                return;
            }

            AppCommon.OutputLogInfo("FIDO認証器とのペアリングを開始します。");
            BluetoothAddress = 0;
            watcher.Start();

            // FIDO認証器がみつかるまで待機（最大10秒）
            AppCommon.OutputLogInfo("FIDO認証器からのアドバタイズ監視を開始します。");
            for (int i = 0; i < 10 && BluetoothAddress == 0; i++) {
                await Task.Run(() => System.Threading.Thread.Sleep(1000));
            }

            watcher.Stop();
            AppCommon.OutputLogInfo("FIDO認証器からのアドバタイズ監視を終了しました。");

            if (BluetoothAddress != 0) {
                // FIDO認証器が見つかった場合はペアリング実行
                FIDOPeripheralFound();
            } else {
                // 画面スレッドに失敗を通知
                FIDOPeripheralPaired(false, AppCommon.MSG_BLE_PARING_ERR_TIMED_OUT);
                AppCommon.OutputLogError("FIDO認証器とのペアリングがタイムアウトしました。");
            }
        }

        private void OnAdvertisementReceived(BluetoothLEAdvertisementWatcher watcher, BluetoothLEAdvertisementReceivedEventArgs eventArgs)
        {
            // FIDO認証器が見つかったら、
            // アドレス情報を保持し、画面スレッドに通知
            string name = eventArgs.Advertisement.LocalName;
            AppCommon.OutputLogInfo(string.Format("BLEデバイス[{0}]が見つかりました: BluetoothAddress={1}, {2} services",
                name, eventArgs.BluetoothAddress, eventArgs.Advertisement.ServiceUuids.Count));
            foreach (Guid g in eventArgs.Advertisement.ServiceUuids) {
                AppCommon.OutputLogDebug(string.Format("  service={0}", g.ToString()));
                if (g.Equals(U2F_BLE_SERVICE_UUID)) {
                    BluetoothAddress = eventArgs.BluetoothAddress;
                    AppCommon.OutputLogInfo("FIDO認証器が見つかりました.");
                    break;
                }
            }
        }

        private void CustomOnPairingRequested(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs args)
        {
            AppCommon.OutputLogInfo("FIDO認証器とのペアリングが自動的に実行されます。");
            args.Accept();
        }

        public async void PairWithFIDOPeripheral()
        {
            bool success = false;
            string messageOnFail = "";
            try {
                // デバイス情報を取得
                BluetoothLEDevice device = await BluetoothLEDevice.FromBluetoothAddressAsync(BluetoothAddress);
                DeviceInformation deviceInfoForPair = device.DeviceInformation;

                // ペアリング実行
                deviceInfoForPair.Pairing.Custom.PairingRequested += CustomOnPairingRequested;
                DevicePairingResult result = await deviceInfoForPair.Pairing.Custom.PairAsync(
                                DevicePairingKinds.ConfirmOnly, 
                                DevicePairingProtectionLevel.Encryption);
                deviceInfoForPair.Pairing.Custom.PairingRequested -= CustomOnPairingRequested;

                // ペアリングが正常終了したら処理完了
                if (result.Status == DevicePairingResultStatus.Paired ||
                    result.Status == DevicePairingResultStatus.AlreadyPaired) {
                    success = true;
                    AppCommon.OutputLogInfo("FIDO認証器とのペアリングが成功しました。");
                } else if (result.Status == DevicePairingResultStatus.Failed) {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_PAIR_MODE;
                    AppCommon.OutputLogError("FIDO認証器とのペアリングが失敗しました。");
                } else {
                    success = false;
                    messageOnFail = AppCommon.MSG_BLE_PARING_ERR_UNKNOWN;
                    AppCommon.OutputLogError(string.Format("FIDO認証器とのペアリングが失敗しました。reason={0}", result.Status));
                }

                // BLEデバイスを解放
                device.Dispose();
                device = null;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.PairWithFIDOPeripheral: {0}", e.Message));
            }

            // 画面スレッドに成否を通知
            FIDOPeripheralPaired(success, messageOnFail);
        }

        public async Task<bool> StartCommunicate()
        {
            critical = false;
            try {
                if (BLEServices.Count == 0) {
                    // サービスをディスカバー
                    if (await DiscoverBLEService() == false) {
                        return false;
                    }
                }

                // データ受信監視を開始
                foreach (GattDeviceService service in BLEServices) {
                    if (await StartBLENotification(service)) {
                        AppCommon.OutputLogInfo(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_START, service.Device.Name));
                        break;
                    }
                    AppCommon.OutputLogError(string.Format("{0}({1})", AppCommon.MSG_BLE_NOTIFICATION_FAILED, service.Device.Name));
                }

                // 接続された場合は true
                return IsConnected();

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.StartCommunicate: {0}", e.Message));
                FreeResources();
                critical = true;
                return false;
            }
        }

        public async Task<bool> DiscoverBLEService()
        {
            critical = false;
            try {
                AppCommon.OutputLogInfo(string.Format("FIDO BLEサービス({0})を検索します。", U2F_BLE_SERVICE_UUID));
                string selector = GattDeviceService.GetDeviceSelectorFromUuid(U2F_BLE_SERVICE_UUID);
                DeviceInformationCollection collection = await DeviceInformation.FindAllAsync(selector);

                foreach (DeviceInformation info in collection) {
                    GattDeviceService service = await GattDeviceService.FromIdAsync(info.Id);
                    if (service != null) {
                        BLEServices.Add(service);
                        AppCommon.OutputLogDebug(string.Format("  FIDO BLE service found [{0}]", info.Name));
                    }
                }

                if (BLEServices.Count == 0) {
                    AppCommon.OutputLogError(AppCommon.MSG_BLE_U2F_SERVICE_NOT_FOUND);
                    return false;
                }

                AppCommon.OutputLogInfo(AppCommon.MSG_BLE_U2F_SERVICE_FOUND);
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.DiscoverBLEService: {0}", e.Message));
                FreeResources();
                critical = true;
                return false;
            }
        }

        public async Task<bool> StartBLENotification(GattDeviceService service)
        {
            critical = false;
            try {
                U2FStatusChar = service.GetCharacteristics(U2F_STATUS_CHAR_UUID)[0];

                GattCommunicationStatus result = await U2FStatusChar.WriteClientCharacteristicConfigurationDescriptorAsync(
                    GattClientCharacteristicConfigurationDescriptorValue.Notify);
                if (result != GattCommunicationStatus.Success) {
                    BLEservice = null;
                    return false;
                }

                U2FControlPointChar = service.GetCharacteristics(U2F_CONTROL_POINT_CHAR_UUID)[0];
                U2FStatusChar.ValueChanged += OnCharacteristicValueChanged;

                // 監視開始したサービスを退避
                BLEservice = service;
                return true;

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.StartBLENotification: {0}", e.Message));
                FreeResources();
                critical = true;
                return false;
            }
        }

        private void OnCharacteristicValueChanged(GattCharacteristic sender, GattValueChangedEventArgs eventArgs)
        {
            try {
                // レスポンスを受領（U2F Statusを読込）
                uint len = eventArgs.CharacteristicValue.Length;
                byte[] responseBytes = new byte[len];
                DataReader.FromBuffer(eventArgs.CharacteristicValue).ReadBytes(responseBytes);

                // レスポンスを転送
                OnDataReceived(responseBytes);

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.OnCharacteristicValueChanged: {0}", e.Message));
                OnTransactionFailed();
            }
        }

        private void StopCommunicate()
        {
            try {
                if (U2FStatusChar != null) {
                    U2FStatusChar.ValueChanged -= OnCharacteristicValueChanged;
                }
                if (BLEservice != null) {
                    BLEservice.Dispose();
                }

            } catch (Exception e) {
                AppCommon.OutputLogError(string.Format("BLEService.StopCommunicate: {0}", e.Message));

            } finally {
                FreeResources();
            }
        }

        public bool IsConnected()
        {
            if (BLEServices.Count == 0) {
                // 接続されていない場合は false
                return false;
            }

            if (BLEservice == null) {
                // データ受信ができない場合は false
                return false;
            }

            // BLE接続されている場合は true
            return true;
        }

        public bool IsCritical()
        {
            return critical;
        }

        private void FreeResources()
        {
            // オブジェクトへの参照を解除
            BLEservice = null;
            U2FStatusChar = null;
        }
    }
}
