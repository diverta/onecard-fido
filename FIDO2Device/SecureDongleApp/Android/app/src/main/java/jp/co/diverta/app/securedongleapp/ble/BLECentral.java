package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.BluetoothAdapter;
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

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    // Bluetooth関連オブジェクトの参照
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeScanner mBleScanner;
    private ScanCallback mScanCallback;

    // タイムアウト監視用
    private Handler mHandler = new Handler();
    private BLECentralScanTimeoutThread mScanTimeoutThread = new BLECentralScanTimeoutThread();

    public BLECentral(MainActivityCommand mac) {
        commandRef = mac;
    }

    public void startScan() {
        // Bluetoothの使用準備
        mBleManager = commandRef.getBluetoothManager();
        mBleAdapter = mBleManager.getAdapter();
        if (mBleAdapter != null && mBleAdapter.isEnabled()) {
            // BLEが使用可能であればスキャンを開始
            startScanDevice();
        }
    }

    public void startScanDevice() {
        // デバイスのスキャンを開始
        Log.d(TAG, "Device scan started");
        mBleScanner = mBleAdapter.getBluetoothLeScanner();
        mScanCallback = new BLECentralScanCallback(this);
        mBleScanner.startScan(mScanCallback);

        // スキャンタイムアウトを監視開始（５秒間）
        mHandler.postDelayed(mScanTimeoutThread, 5000);
    }

    public void stopScanDevice() {
        // スキャンタイムアウト時のコールバックを削除
        mHandler.removeCallbacks(mScanTimeoutThread);

        // デバイスのスキャンを停止
        mBleScanner.stopScan(mScanCallback);
        Log.d(TAG, "Device scan stopped");

        // TODO: 仮の実装です。
        // コマンドクラスに制御を戻す
        commandRef.onBLEConnectionTerminated(true);
    }

    private class BLECentralScanTimeoutThread implements Runnable
    {
        @Override
        public void run() {
            // タイムアウトが発生した場合、スキャンを停止
            Log.d(TAG, "Device scan timed out");
            stopScanDevice();
        }
    }
}
