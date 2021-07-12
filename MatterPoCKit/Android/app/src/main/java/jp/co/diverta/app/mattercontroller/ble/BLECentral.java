package jp.co.diverta.app.mattercontroller.ble;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothManager;
import android.bluetooth.le.BluetoothLeScanner;
import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanFilter;
import android.bluetooth.le.ScanSettings;
import android.os.Handler;
import android.os.ParcelUuid;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

import chip.devicecontroller.ChipDeviceController;
import jp.co.diverta.app.mattercontroller.ChipCompletionListener;
import jp.co.diverta.app.mattercontroller.ChipDeviceIdUtil;
import jp.co.diverta.app.mattercontroller.MainActivityCommand;
import jp.co.diverta.app.mattercontroller.R;

public class BLECentral
{
    // ログ表示用
    private String TAG = getClass().getName();

    public static final String CHIP_UUID = "0000FFF6-0000-1000-8000-00805F9B34FB";

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    // Bluetooth関連オブジェクトの参照
    private BluetoothManager mBleManager;
    private BluetoothAdapter mBleAdapter;
    private BluetoothLeScanner mBleScanner;
    private ScanCallback mScanCallback;
    private BluetoothGattCallback mGattCallback;
    private BluetoothGatt mGatt;

    // タイムアウト監視用
    private Handler mHandler = new Handler();
    private BLECentralScanTimeoutThread mScanTimeoutThread = new BLECentralScanTimeoutThread();

    // CHIPクライアント
    ChipDeviceController mDeviceController = null;
    public ChipDeviceController getDeviceController() {
        return mDeviceController;
    }

    public BLECentral(MainActivityCommand mac) {
        commandRef = mac;
    }

    //
    // デバイススキャン関連処理
    //
    public void startScan(ChipDeviceController deviceController) {
        // ChipDeviceControllerの参照を保持
        mDeviceController = deviceController;

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
        // デバイススキャン設定
        mBleScanner = mBleAdapter.getBluetoothLeScanner();
        ParcelUuid uuidCHIP = ParcelUuid.fromString(CHIP_UUID);
        byte[] serviceData = {0x00, 0x00, 0x0f};
        ScanFilter scanFilter = new ScanFilter.Builder().setServiceData(uuidCHIP, serviceData).build();
        List<ScanFilter> scanFilterList = new ArrayList<ScanFilter>();
        scanFilterList.add(scanFilter);
        ScanSettings scanSettings = new ScanSettings.Builder().setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY).build();

        // デバイスのスキャンを開始
        mScanCallback = new BLECentralScanCallback(this);
        mBleScanner.startScan(scanFilterList, scanSettings, mScanCallback);
        Log.d(TAG, "Device scan started");

        // スキャンタイムアウトを監視開始（10秒間）
        mHandler.postDelayed(mScanTimeoutThread, 10000);
    }

    public void onDeviceScanned(BluetoothDevice device) {
        // デバイスのスキャンが成功した場合、スキャンを停止
        stopScanDevice();

        // デバイスに接続
        mGattCallback = new BLECentralGattCallback(this);
        mGatt = device.connectGatt(commandRef.getApplicationContext(), false, mGattCallback);
    }

    public void onDeviceConnected() {
        // ペアリングを実行
        performBlePairing();
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
            Log.e(TAG, "Device scan timed out");
            stopScanDevice();
            // コマンドクラスに制御を戻す
            commandRef.appendResourceStringMessage(R.string.msg_pairing_scan_timeout);
            commandRef.onBLEConnectionTerminated(false);
        }
    }

    //
    // BLEペアリング関連処理
    //
    private void performBlePairing() {
        // コールバックを設定
        ChipDeviceController.CompletionListener completionListener = new ChipCompletionListener(this);
        mDeviceController.setCompletionListener(completionListener);

        // BLEペアリングを実行
        mDeviceId = ChipDeviceIdUtil.getNextAvailableId(commandRef.getApplicationContext());
        long setupPinCode = 20202021;
        mDeviceController.pairDevice(mGatt, mDeviceId, setupPinCode);
    }

    public void onBLEPairingCompleted(boolean success) {
        if (success) {
            // BLEペアリング成功時はコミッショニング処理に移行
            Log.i(TAG, String.format("BLE pairing completed with Matter device: deviceId=%d", mDeviceId));
            performCommissioning();

        } else {
            // BLEペアリング失敗時は切断
            mGatt.disconnect();
            mGatt.close();
            // コマンドクラスに制御を戻す
            Log.e(TAG, "BLE pairing failed");
            commandRef.appendResourceStringMessage(R.string.msg_pairing_ble_fail);
            commandRef.onBLEConnectionTerminated(false);
        }
    }

    // ペアリング先のデバイスIDを保持
    private long mDeviceId;

    //
    // コミッショニング関連処理
    //
    private void performCommissioning() {
        // コミッショニング先の情報
        int channel = 15;
        int panId = 0x1234;
        byte[] extendedPanId = {0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22};
        byte[] masterKey = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, (byte)0x88, (byte)0x99, (byte)0xaa, (byte)0xbb, (byte)0xcc, (byte)0xdd, (byte)0xee, (byte)0xff};

        // Operational datasetを生成
        byte[] dataset = makeOperationalDataset(channel, panId, extendedPanId, masterKey);

        // コミッショニング処理を実行
        mDeviceController.enableThreadNetwork(mDeviceId, dataset);
    }

    public void onCommissioningComplete(boolean success) {
        if (success) {
            // 次回のコミッショニング時に使用するデバイスIDを更新
            ChipDeviceIdUtil.setNextAvailableId(commandRef.getApplicationContext(), mDeviceId + 1);

            // コミッショニング成功時
            Log.i(TAG, "Network commissioning completed with Matter hub");
            commandRef.onBLEConnectionTerminated(true);

        } else {
            // コミッショニング失敗時
            mGatt.disconnect();
            mGatt.close();
            Log.e(TAG, "Network commissioning failed");
            commandRef.appendResourceStringMessage(R.string.msg_pairing_commission_fail);
            commandRef.onBLEConnectionTerminated(false);
        }
    }

    //
    // Operational Dataset生成関連
    //
    private byte TYPE_CHANNEL = 0;
    private byte TYPE_PANID = 1;
    private byte TYPE_XPANID = 2;
    private byte TYPE_MASTER_KEY = 5;
    private byte NUM_CHANNEL_BYTES = 3;
    private byte NUM_PANID_BYTES = 2;
    private byte NUM_XPANID_BYTES = 8;
    private byte NUM_MASTER_KEY_BYTES = 16;

    private byte[] makeOperationalDataset(int channel, int panId, byte[] extendedPanId, byte[] masterKey) {
        // Operational DatasetをTLV形式で生成
        int size = 8 + NUM_CHANNEL_BYTES + NUM_PANID_BYTES + NUM_XPANID_BYTES + NUM_MASTER_KEY_BYTES;
        byte[] dataset = new byte[size];

        // channel
        int offset = 0;
        dataset[offset++] = TYPE_CHANNEL;
        dataset[offset++] = NUM_CHANNEL_BYTES;
        dataset[offset++] = 0x00;
        dataset[offset++] = (byte)(channel >> 8 & 0xff);
        dataset[offset++] = (byte)(channel & 0xff);

        // pan ID
        dataset[offset++] = TYPE_PANID;
        dataset[offset++] = NUM_PANID_BYTES;
        dataset[offset++] = (byte)(panId >> 8 & 0xff);
        dataset[offset++] = (byte)(panId & 0xff);

        // extended pan ID
        dataset[offset++] = TYPE_XPANID;
        dataset[offset++] = NUM_XPANID_BYTES;
        for (int i = 0; i < NUM_XPANID_BYTES; i++) {
            dataset[offset++] = extendedPanId[i];
        }

        // master key
        dataset[offset++] = TYPE_MASTER_KEY;
        dataset[offset++] = NUM_MASTER_KEY_BYTES;
        for (int i = 0; i < NUM_MASTER_KEY_BYTES; i++) {
            dataset[offset++] = masterKey[i];
        }

        return dataset;
    }
}
