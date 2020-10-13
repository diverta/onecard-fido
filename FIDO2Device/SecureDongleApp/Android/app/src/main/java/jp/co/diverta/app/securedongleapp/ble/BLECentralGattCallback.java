package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothProfile;

public class BLECentralGattCallback extends BluetoothGattCallback
{
    // ログ表示用
    private String TAG = getClass().getName();

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;
    private BLECentralU2FService mU2FService;

    public BLECentralGattCallback(BLECentral bc) {
        centralRef = bc;
    }

    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
        super.onConnectionStateChange(gatt, status, newState);

        // 接続状況が変化したら実行.
        switch (newState) {
            case BluetoothProfile.STATE_CONNECTED:
                centralRef.onDeviceStateConnected(gatt);
                break;
            case BluetoothProfile.STATE_DISCONNECTED:
                centralRef.onDeviceStateDisconnected();
                break;
            default:
                break;
        }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status)
    {
        // BLE U2Fサービスオブジェクトのインスタンスを生成
        mU2FService = new BLECentralU2FService(gatt);
        if (status != BluetoothGatt.GATT_SUCCESS) {
            // GATTサービスが正常に検索できなかった時は制御を戻す
            centralRef.onServicesDiscovered(false, mU2FService);
            return;
        }

        // U2Fサービスを検索
        if (mU2FService.getService() == false) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, mU2FService);
            return;
        }

        // U2Fキャラクタリスティックを検索
        if (mU2FService.getCharacteristics() == false) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, mU2FService);
            return;
        }

        // U2F statusのNotificationを有効化
        if (mU2FService.prepareNotification() == false) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, mU2FService);
            return;
        }

        // 接続完了
        centralRef.onServicesDiscovered(true, mU2FService);
    }
}
