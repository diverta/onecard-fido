package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.util.Log;

import java.util.UUID;

public class BLECentralGattCallback extends BluetoothGattCallback
{
    // ログ表示用
    private String TAG = getClass().getName();

    // UUID関連
    private static final String CHARACTERISTIC_CONFIG_UUID = "00002902-0000-1000-8000-00805f9b34fb";

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;

    // キャラクタリスティックの参照を保持
    private BluetoothGattCharacteristic mU2fStatusChar;
    private BluetoothGattCharacteristic mU2fControlPointChar;

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
        if (status != BluetoothGatt.GATT_SUCCESS) {
            // GATTサービスが正常に検索できなかった時は制御を戻す
            centralRef.onServicesDiscovered(false, gatt);
            return;
        }

        // U2Fサービスを検索
        BluetoothGattService service = gatt.getService(UUID.fromString(BLECentral.U2F_SERVICE_UUID));
        if (service == null) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, gatt);
            return;
        }
        Log.d(TAG, "U2F service found");

        // U2F command キャラクタリスティックを検索
        mU2fControlPointChar = service.getCharacteristic(UUID.fromString(BLECentral.U2F_CONTROL_POINT_CHAR_UUID));
        if (mU2fControlPointChar == null) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, gatt);
            return;
        }
        Log.d(TAG, "U2F command characteristic found");

        // U2F response キャラクタリスティックを検索
        mU2fStatusChar = service.getCharacteristic(UUID.fromString(BLECentral.U2F_STATUS_CHAR_UUID));
        if (mU2fStatusChar == null) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, gatt);
            return;
        }
        Log.d(TAG, "U2F response characteristic found");

        // U2F statusのNotificationを有効化
        if (prepareNotification(gatt) == false) {
            // 失敗時は制御を戻す
            centralRef.onServicesDiscovered(false, gatt);
            return;
        }
        Log.d(TAG, "U2F response notification ready");

        // 接続完了
        centralRef.onServicesDiscovered(true, gatt);
    }

    private boolean prepareNotification(BluetoothGatt gatt) {
        // U2F statusのNotificationを有効化
        if (gatt.setCharacteristicNotification(mU2fStatusChar, true) == false) {
            return false;
        }
        BluetoothGattDescriptor descriptor = mU2fStatusChar.getDescriptor(UUID.fromString(CHARACTERISTIC_CONFIG_UUID));
        descriptor.setValue(BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE);
        if (gatt.writeDescriptor(descriptor) == false) {
            return false;
        }
        return true;
    }
}
