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
}
