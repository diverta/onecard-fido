package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.os.Handler;
import android.util.Log;

import jp.co.diverta.app.securedongleapp.MainActivityCommand;

public class BLECentral
{
    // ログ表示用
    private String TAG = getClass().getName();

    // UUID関連
    public static final String U2F_SERVICE_UUID            = "0000FFFD-0000-1000-8000-00805F9B34FB";
    public static final String U2F_CONTROL_POINT_CHAR_UUID = "F1D0FFF1-DEAA-ECEE-B42F-C9BA7ED623BB";
    public static final String U2F_STATUS_CHAR_UUID        = "F1D0FFF2-DEAA-ECEE-B42F-C9BA7ED623BB";

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    // Bluetooth関連オブジェクトの参照
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeScanner mBleScanner;
    private BluetoothDevice mBleDevice;
    private BluetoothGatt mBleGatt;
    private ScanCallback mScanCallback;
    private BluetoothGattCallback mGattCallback;

    // タイムアウト監視用
    private Handler mHandler = new Handler();
    private BLECentralScanTimeoutThread mScanTimeoutThread = new BLECentralScanTimeoutThread();
    private BLEConnectionTimeoutThread mConnectionTimeoutThread = new BLEConnectionTimeoutThread();

    public BLECentral(MainActivityCommand mac) {
        commandRef = mac;
    }

    //
    // デバイススキャン関連処理
    //

    public void startScan() {
        // Bluetoothの使用準備
        mBleManager = commandRef.getBluetoothManager();
        mBleAdapter = mBleManager.getAdapter();
        if (mBleAdapter != null && mBleAdapter.isEnabled()) {
            // BLEが使用可能であればスキャンを開始
            startScanDevice();
        } else {
            // Bluetoothがオフの場合はエラー
            commandRef.cannotStartBLEConnection();
        }
    }

    private void startScanDevice() {
        // デバイスのスキャンを開始
        Log.d(TAG, "Device scan started");
        mBleScanner = mBleAdapter.getBluetoothLeScanner();
        mScanCallback = new BLECentralScanCallback(this);
        mBleScanner.startScan(mScanCallback);

        // スキャンタイムアウトを監視開始（５秒間）
        mHandler.postDelayed(mScanTimeoutThread, 5000);
    }

    public void onDeviceScanned(BluetoothDevice device) {
        // デバイスのスキャンが成功した場合、スキャンを停止
        stopScanDevice();
        // スキャンされたデバイスに接続
        connectDevice(device);
    }

    private void stopScanDevice() {
        // スキャンタイムアウト時のコールバックを削除
        mHandler.removeCallbacks(mScanTimeoutThread);

        // デバイスのスキャンを停止
        mBleScanner.stopScan(mScanCallback);
        Log.d(TAG, "Device scan stopped");
    }

    private class BLECentralScanTimeoutThread implements Runnable
    {
        @Override
        public void run() {
            // タイムアウトが発生した場合、スキャンを停止
            Log.d(TAG, "Device scan timed out");
            stopScanDevice();
            // コマンドクラスに制御を戻す
            commandRef.onBLEConnectionTerminated(false);
        }
    }

    //
    // デバイス接続関連処理
    //

    private void connectDevice(BluetoothDevice device) {
        // スキャンされたデバイスに接続
        mGattCallback = new BLECentralGattCallback(this);
        device.connectGatt(commandRef.getApplicationContext(), false, mGattCallback);
        // 接続タイムアウトを監視開始（60秒間）
        mHandler.postDelayed(mConnectionTimeoutThread, 60000);
    }

    public void onDeviceStateConnected(BluetoothGatt gatt) {
        Log.d(TAG, "Device connected");

        // 接続オブジェクトを保持
        mBleGatt = gatt;
    }

    public void onDeviceStateDisconnected() {
        Log.d(TAG, "Device disconnected");

        // 接続オブジェクトを閉じる
        closeBleGatt();
        // コマンドクラスに制御を戻す
        commandRef.onBLEConnectionTerminated(true);
    }

    private void closeBleGatt() {
        // 接続タイムアウト時のコールバックを削除
        mHandler.removeCallbacks(mConnectionTimeoutThread);

        // 接続オブジェクトを閉じる
        if (mBleGatt != null) {
            mBleGatt.close();
            mBleGatt = null;
            Log.d(TAG, "BLE gatt object closed");
        }
    }

    private class BLEConnectionTimeoutThread implements Runnable
    {
        @Override
        public void run() {
            // タイムアウトが発生した場合、接続を停止
            Log.d(TAG, "Device connection timed out");
            closeBleGatt();
            // コマンドクラスに制御を戻す
            commandRef.onBLEConnectionTerminated(false);
        }
    }
}
