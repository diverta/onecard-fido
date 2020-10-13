package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.util.Log;

import java.util.UUID;

public class BLECentralU2FService
{
    // ログ表示用
    private String TAG = getClass().getName();

    // UUID関連
    public static final String U2F_SERVICE_UUID            = "0000FFFD-0000-1000-8000-00805F9B34FB";
    public static final String U2F_CONTROL_POINT_CHAR_UUID = "F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB";
    public static final String U2F_STATUS_CHAR_UUID        = "F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB";
    public static final String CHARACTERISTIC_CONFIG_UUID  = "00002902-0000-1000-8000-00805f9b34fb";

    // Bluetooth関連オブジェクトの参照
    private BluetoothGatt mBleGatt;
    private BluetoothGattService mBleGattService;

    // キャラクタリスティックの参照を保持
    private BluetoothGattCharacteristic mU2fStatusChar;
    private BluetoothGattCharacteristic mU2fControlPointChar;

    public BLECentralU2FService(BluetoothGatt gatt) {
        mBleGatt = gatt;
    }

    //
    // オブジェクト参照の取得
    //

    public BluetoothGatt getBleGattRef() {
        return mBleGatt;
    }

    public BluetoothGattService getBleGattServiceRef() {
        return mBleGattService;
    }

    //
    // サービス／キャラクタリスティック関連処理
    //

    public boolean getService() {
        mBleGattService = mBleGatt.getService(UUID.fromString(U2F_SERVICE_UUID));
        if (mBleGattService == null) {
            return false;
        }
        Log.d(TAG, "U2F service found");
        return true;
    }

    public boolean getCharacteristics() {
        // U2F command キャラクタリスティックを検索
        mU2fControlPointChar = mBleGattService.getCharacteristic(UUID.fromString(U2F_CONTROL_POINT_CHAR_UUID));
        if (mU2fControlPointChar == null) {
            // 失敗時は制御を戻す
            return false;
        }
        Log.d(TAG, "U2F command characteristic found");

        // U2F response キャラクタリスティックを検索
        mU2fStatusChar = mBleGattService.getCharacteristic(UUID.fromString(U2F_STATUS_CHAR_UUID));
        if (mU2fStatusChar == null) {
            // 失敗時は制御を戻す
            return false;
        }
        Log.d(TAG, "U2F response characteristic found");
        return true;
    }

    public boolean prepareNotification() {
        // U2F statusのNotificationを有効化
        if (mBleGatt.setCharacteristicNotification(mU2fStatusChar, true) == false) {
            return false;
        }
        BluetoothGattDescriptor descriptor = mU2fStatusChar.getDescriptor(UUID.fromString(CHARACTERISTIC_CONFIG_UUID));
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        if (mBleGatt.writeDescriptor(descriptor) == false) {
            return false;
        }
        Log.d(TAG, "Response notification ready");
        return true;
    }
}
