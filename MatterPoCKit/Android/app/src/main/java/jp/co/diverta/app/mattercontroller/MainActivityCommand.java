package jp.co.diverta.app.mattercontroller;

import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.net.nsd.NsdServiceInfo;
import android.os.Message;
import android.util.Log;

import chip.devicecontroller.ChipClusters;
import chip.devicecontroller.ChipClusters.OnOffCluster;
import jp.co.diverta.app.mattercontroller.ble.BLECentral;
import jp.co.diverta.app.mattercontroller.chip.ClusterCallback;

public class MainActivityCommand
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private BLECentral bleCentral;

    // 別スレッドから画面操作するためのハンドラー
    private MainActivityGUIHandler handlerRef;

    // Matterデバイス検索済みフラグ
    //   初回ペアリング／コミッショニング、または
    //   Address Updateを実施済みかどうかを保持
    private boolean mDeviceDiscovered = false;

    // ChipClient、ChipServiceResolverの参照を保持
    private ChipClient mChipClient;
    private ChipServiceResolver mServiceResolver;

    // ログ表示用
    private String TAG = getClass().getName();

    public MainActivityCommand(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
        handlerRef = new MainActivityGUIHandler(guiRef);
        bleCentral = new BLECentral(this);
        mChipClient = new ChipClient(this);
        mServiceResolver = new ChipServiceResolver(this);

        // デバイス検索ボタンを不活性化
        // TODO: 仮の実装です。
        setButtonUpdateAddressEnabled(false);

        // アドレス更新済みフラグをクリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(false);
    }

    public void startBLEConnection() {
        // 処理開始ログ
        Log.d(TAG, "Matter device commissioning start");
        // アドレス更新済みフラグをクリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(false);
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        displayStatusText(getResourceString(R.string.msg_pairing_will_start));
        // スキャンを開始
        bleCentral.startScan(mChipClient.getDeviceController());
    }

    public void cannotStartBLEConnection() {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // アドレス更新済みフラグをクリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(false);
        // BLEが無効の場合
        displayStatusText(getResourceString(R.string.msg_bluetooth_is_turned_off));
    }

    public void onBLEConnectionTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // アドレス更新済みフラグを設定／クリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(success);
        if (success) {
            appendStatusText(getResourceString(R.string.msg_pairing_success));
            Log.d(TAG, "Matter device commissioning success");
        } else {
            appendStatusText(getResourceString(R.string.msg_pairing_failure));
            Log.d(TAG, "Matter device commissioning failed");
        }
    }

    private void setDeviceDiscovered(boolean discovered) {
        // アドレス更新済みフラグを設定／クリア
        mDeviceDiscovered = discovered;
        // コマンド実行ボタンを押下可／不可に設定
        setButtonDoCommandEnabled(mDeviceDiscovered);
    }

    //
    // Update address
    //
    public void startUpdateAddress() {
        // 処理開始ログ
        Log.d(TAG, "Matter device address update start");
        // アドレス更新済みフラグをクリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(false);
        displayStatusText(getResourceString(R.string.msg_update_address_will_start));
        // Matterデバイス検索処理を開始
        mServiceResolver.startResolve();
    }

    public void onChipServiceResolved(boolean success, NsdServiceInfo serviceInfo) {
        // 検索処理が失敗した場合は終了
        if (success == false) {
            onUpdateAddressTerminated(false);
            return;
        }

        // 検索されたデバイスのIPv6アドレス／ポート番号を取得
        String hostAddress = serviceInfo.getHost().getHostAddress();
        int port = serviceInfo.getPort();
        Log.d(TAG, String.format("Matter device resolved: Host=%s Port=%d", hostAddress, port));
        long deviceId = ChipDeviceIdUtil.getLastDeviceId(getApplicationContext());

        // ChipDeviceControllerのアドレス情報を更新
        //   同期型の処理なので、コールバック設定は不要
        try {
            mChipClient.getDeviceController().updateAddress(deviceId, hostAddress, port);
            onUpdateAddressTerminated(true);

        } catch (Exception e) {
            Log.d(TAG, String.format("Matter device address update fail: %s", e));
            onUpdateAddressTerminated(false);
        }
    }

    public void onUpdateAddressTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // アドレス更新済みフラグを設定／クリア（コマンド実行ボタンも制御）
        setDeviceDiscovered(success);
        if (success) {
            appendStatusText(getResourceString(R.string.msg_update_address_success));
            Log.d(TAG, "Matter device address update completed");
        } else {
            appendStatusText(getResourceString(R.string.msg_update_address_failure));
            Log.d(TAG, "Matter device address update not completed");
        }
    }

    //
    // Matterコマンドを実行するための関数群
    //
    public void performOffCommand() {
        // メッセージを表示
        displayStatusText(getResourceString(R.string.msg_off_command_will_start));
        Log.d(TAG, "Matter device off command start");

        // デバイス参照を取得
        long devicePtr = getActiveDevicePtr();

        // Offコマンドを実行
        int endpoint = 1;
        ChipClusters.OnOffCluster onOffCluster = new OnOffCluster(devicePtr, endpoint);
        onOffCluster.off(new ClusterCallback(this));
    }

    public void performOnCommand() {
        // メッセージを表示
        displayStatusText(getResourceString(R.string.msg_on_command_will_start));
        Log.d(TAG, "Matter device on command start");

        // デバイス参照を取得
        long devicePtr = getActiveDevicePtr();

        // Onコマンドを実行
        int endpoint = 1;
        ChipClusters.OnOffCluster onOffCluster = new OnOffCluster(devicePtr, endpoint);
        onOffCluster.on(new ClusterCallback(this));
    }

    private long getActiveDevicePtr() {
        // 最後にコミッショニングを完了したデバイスIDから、
        // デバイス参照を取得
        long deviceId = ChipDeviceIdUtil.getLastDeviceId(getApplicationContext());
        long devicePtr = mChipClient.getDeviceController().getDevicePointer(deviceId);
        return devicePtr;
    }

    public void onOnOffCommandTerminated(boolean success) {
        // 終了メッセージを表示
        if (success) {
            appendStatusText(getResourceString(R.string.msg_perform_command_success));
            Log.d(TAG, "Matter device on/off command success");
        } else {
            appendStatusText(getResourceString(R.string.msg_perform_command_failure));
            Log.d(TAG, "Matter device on/off command failed");
        }
    }

    //
    // MainActivityにアクセスするための関数群
    //
    public void displayStatusText(String s) {
        // ステータス表示欄に文字列を表示
        Message msg = Message.obtain(handlerRef, MainActivityGUIHandler.DISPLAY_TEXT, s);
        handlerRef.sendMessage(msg);
    }

    public void appendStatusText(String s) {
        // ステータス表示欄に文字列を追加表示
        Message msg = Message.obtain(handlerRef, MainActivityGUIHandler.APPEND_TEXT, s);
        handlerRef.sendMessage(msg);
    }

    public void setButtonsEnabled(boolean enable) {
        if (enable) {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTONS_ENABLE);
        } else {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTONS_DISABLE);
        }
    }

    public void setButtonDoCommandEnabled(boolean enable) {
        if (enable) {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_DOCMD_ENABLE);
        } else {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_DOCMD_DISABLE);
        }
    }

    public void setButtonUpdateAddressEnabled(boolean enable) {
        if (enable) {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_UPADR_ENABLE);
        } else {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_UPADR_DISABLE);
        }
    }

    public BluetoothManager getBluetoothManager() {
        return (BluetoothManager)guiRef.getSystemService(Context.BLUETOOTH_SERVICE);
    }

    //
    // ユーティリティー関数
    //
    public Context getApplicationContext() {
        return guiRef.getApplicationContext();
    }

    public String getResourceString(int resId) {
        return guiRef.getString(resId);
    }

    public void appendResourceStringMessage(int resId) {
        String msg = getResourceString(resId);
        appendStatusText(msg);
    }
}
