package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.util.Log;

import jp.co.diverta.app.securedongleapp.MainActivityCommand;

public class BLECentral
{
    // ログ表示用
    private String TAG = getClass().getName();

    // メイン画面の参照を保持
    private MainActivityCommand commandRef;

    // Bluetooth関連オブジェクトの参照
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;

    public BLECentral(MainActivityCommand mac) {
        commandRef = mac;
    }

    public void startScan() {
        // Bluetoothの使用準備
        mBleManager = commandRef.getBluetoothManager();
        mBleAdapter = mBleManager.getAdapter();
        if (mBleAdapter != null && mBleAdapter.isEnabled()) {
            // BLEが使用可能であればスキャンを開始
            scanDevice();
        }
    }

    public void scanDevice() {
        // TODO: 後日正式に実装予定
        Log.d(TAG, "Device scan started");
    }
}
