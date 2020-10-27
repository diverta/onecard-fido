package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGattServer;
import android.bluetooth.BluetoothGattServerCallback;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.le.AdvertiseCallback;
import android.bluetooth.le.AdvertiseData;
import android.bluetooth.le.AdvertiseSettings;
import android.bluetooth.le.BluetoothLeAdvertiser;
import android.content.Context;
import android.os.ParcelUuid;
import android.util.Log;

import jp.co.diverta.app.securedongleapp.MainActivityCommand;
import jp.co.diverta.app.securedongleapp.R;

public class BLEPeripheral
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    // アドバタイジングUUID
    public static final String BLE_ADVERTISE_UUID = "422E0000-E141-11E5-A837-0800200C9A66";

    // Bluetooth関連オブジェクトの参照
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeAdvertiser mBtAdvertiser;
    private BLEAdvertiseCallback advertiseStartCallback = new BLEAdvertiseCallback();
    private BluetoothGattServer mBtGattServer;
    private BLEGattServerCallback mGattServerCallback = new BLEGattServerCallback();;

    public BLEPeripheral(MainActivityCommand mac) {
        commandRef = mac;
    }

    //
    // アドバタイズ関連処理
    //

    public void startBLEAdvertise() {
        // BLEペリフェラルの初期化
        if (prepareBle() == false) {
            // コマンドクラスに制御を戻す
            commandRef.onBLEAdvertiseCallback(false);
            return;
        }

        // BLEペリフェラルとしてアドバタイジングを開始
        startBleAdvertising();
    }

    public void stopBLEAdvertise() {
        // アドバタイジングを停止
        mBtAdvertiser.stopAdvertising(advertiseStartCallback);
        mBtGattServer.close();
        Log.d(TAG, "Advertising will stop");
    }

    //
    // 内部処理
    //

    private boolean prepareBle() {
        Context context = commandRef.getApplicationContext();
        mBleManager = (BluetoothManager)context.getSystemService(Context.BLUETOOTH_SERVICE);
        mBleAdapter = mBleManager.getAdapter();
        if (mBleAdapter == null) {
            Log.e(TAG, "BluetoothManager.getAdapter() returns null");
            commandRef.appendResourceStringMessage(R.string.msg_ble_peripheral_mode_unavailable);
            return false;
        }
        Log.d(TAG, "Ready to prepare BLE peripheral");

        mBtAdvertiser = mBleAdapter.getBluetoothLeAdvertiser();
        if (mBtAdvertiser == null) {
            Log.e(TAG, "BluetoothAdapter.getBluetoothLeAdvertiser() returns null");
            commandRef.appendResourceStringMessage(R.string.msg_ble_advertise_unavailable);
            return false;
        }

        // ドングルからの接続／切断検知用ハンドラーを設定
        mBtGattServer = mBleManager.openGattServer(context, mGattServerCallback);

        // 0.2秒 wait
        commandRef.waitMilliSeconds(200);
        Log.d(TAG, "Ready to start BLE advertising");
        return true;
    }

    private void startBleAdvertising() {
        AdvertiseData.Builder dataBuilder = new AdvertiseData.Builder();
        dataBuilder.setIncludeTxPowerLevel(true);
        dataBuilder.addServiceUuid(ParcelUuid.fromString(BLE_ADVERTISE_UUID));

        AdvertiseSettings.Builder settingsBuilder = new AdvertiseSettings.Builder();
        settingsBuilder.setAdvertiseMode(AdvertiseSettings.ADVERTISE_MODE_LOW_LATENCY);
        settingsBuilder.setTxPowerLevel(AdvertiseSettings.ADVERTISE_TX_POWER_HIGH);

        AdvertiseData.Builder respBuilder = new AdvertiseData.Builder();
        respBuilder.setIncludeDeviceName(true);

        mBtAdvertiser.startAdvertising(settingsBuilder.build(), dataBuilder.build(), respBuilder.build(), advertiseStartCallback);
    }

    private class BLEAdvertiseCallback extends AdvertiseCallback
    {
        @Override
        public void onStartSuccess(AdvertiseSettings settingsInEffect) {
            super.onStartSuccess(settingsInEffect);
            Log.d(TAG, "Advertisement start success");
            // コマンドクラスに制御を戻す
            commandRef.onBLEAdvertiseCallback(true);
        }

        @Override
        public void onStartFailure(int errorCode) {
            super.onStartFailure(errorCode);
            Log.d(TAG, "Advertisement start fail");
            commandRef.appendResourceStringMessage(R.string.msg_ble_advertise_start_fail);
            // コマンドクラスに制御を戻す
            commandRef.onBLEAdvertiseCallback(false);
        }
    }

    private class BLEGattServerCallback extends BluetoothGattServerCallback
    {
        @Override
        public void onConnectionStateChange(BluetoothDevice device, int status, int newState) {
            super.onConnectionStateChange(device, status, newState);
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                // 接続が確立されたら５秒後に切断させるようにする
                Log.d(TAG, "Connection state changed to connected(" + device.toString() + "): status=" + status);
                commandRef.onBLEGattServerCallback();

            } else {
                Log.d(TAG, "Connection state changed to disconnected(" + device.toString() + "): status=" + status);
            }
        }
    }
}
