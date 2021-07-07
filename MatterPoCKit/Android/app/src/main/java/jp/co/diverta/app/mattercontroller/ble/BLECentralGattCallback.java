package jp.co.diverta.app.mattercontroller.ble;

import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothProfile;
import android.util.Log;

public class BLECentralGattCallback extends BluetoothGattCallback
{
    // ログ表示用
    private String TAG = getClass().getName();

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;
    private BluetoothGattCallback wrappedCallback;

    public BLECentralGattCallback(BLECentral bc) {
        centralRef = bc;
        wrappedCallback = centralRef.getDeviceController().getCallback();
    }

    @Override
    public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
        if (newState == BluetoothProfile.STATE_CONNECTED && status == BluetoothGatt.GATT_SUCCESS) {
            wrappedCallback.onConnectionStateChange(gatt, status, newState);

            // 正常接続時はサービス検索に移行
            Log.d(TAG, "Discovering Services...");
            gatt.discoverServices();
        }
    }

    @Override
    public void onServicesDiscovered(BluetoothGatt gatt, int status) {
        wrappedCallback.onServicesDiscovered(gatt, status);

        if (status == BluetoothGatt.GATT_SUCCESS) {
            // 正常接続時はMTUリクエストに移行
            Log.d(TAG, "Services Discovered");
            gatt.requestMtu(131);
        }
    }

    @Override
    public void onMtuChanged(BluetoothGatt gatt, int mtu, int status) {
        wrappedCallback.onMtuChanged(gatt, mtu, status);

        if (status == BluetoothGatt.GATT_SUCCESS) {
            // 正常接続時は後続処理を実行
            Log.d(TAG, "MTU changed");
            centralRef.onDeviceConnected();
        }
    }

    @Override
    public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
        wrappedCallback.onCharacteristicRead(gatt, characteristic, status);
    }

    @Override
    public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status) {
        wrappedCallback.onCharacteristicWrite(gatt, characteristic, status);
    }

    @Override
    public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic) {
        wrappedCallback.onCharacteristicChanged(gatt, characteristic);
    }

    @Override
    public void onDescriptorRead(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        wrappedCallback.onDescriptorRead(gatt, descriptor, status);
    }

    @Override
    public void onDescriptorWrite(BluetoothGatt gatt, BluetoothGattDescriptor descriptor, int status) {
        wrappedCallback.onDescriptorWrite(gatt, descriptor, status);
    }

    @Override
    public void onReliableWriteCompleted(BluetoothGatt gatt, int status) {
        wrappedCallback.onReliableWriteCompleted(gatt, status);
    }

    @Override
    public void onReadRemoteRssi(BluetoothGatt gatt, int rssi, int status) {
        wrappedCallback.onReadRemoteRssi(gatt, rssi, status);
    }
}
