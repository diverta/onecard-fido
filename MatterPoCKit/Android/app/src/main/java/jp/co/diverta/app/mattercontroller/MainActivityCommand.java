package jp.co.diverta.app.mattercontroller;

import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.net.nsd.NsdServiceInfo;
import android.os.Message;
import android.util.Log;

import jp.co.diverta.app.mattercontroller.ble.BLECentral;

public class MainActivityCommand
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private MainActivityGUIHandler handlerRef;
    private BLECentral bleCentral;

    // アドレス更新済みフラグ
    //   Address Updateを実施済みかどうかを保持
    private boolean mAddressUpdated = false;

    // ChipClientの参照を保持
    private ChipClient mChipClient = null;

    // ログ表示用
    private String TAG = getClass().getName();

    public MainActivityCommand(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
        handlerRef = guiRef.guiHandler;
        bleCentral = new BLECentral(this);
        mChipClient = new ChipClient(this);

        // コマンド実行ボタンを押下不可に設定
        setButtonDoCommandEnabled(false);
    }

    public void startBLEConnection() {
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        displayStatusText(getResourceString(R.string.msg_pairing_will_start));
        // スキャンを開始
        bleCentral.startScan(mChipClient.getDeviceController());
    }

    public void cannotStartBLEConnection() {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // コマンド実行ボタンを押下不可に設定
        setButtonDoCommandEnabled(false);
        // BLEが無効の場合
        displayStatusText(getResourceString(R.string.msg_bluetooth_is_turned_off));
    }

    public void onBLEConnectionTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // コマンド実行ボタンを押下不可に設定
        setButtonDoCommandEnabled(false);
        if (success) {
            appendStatusText(getResourceString(R.string.msg_pairing_success));
        } else {
            appendStatusText(getResourceString(R.string.msg_pairing_failure));
        }
    }

    //
    // Update address
    //
    public void startUpdateAddress() {
        // アドレス更新済みフラグをクリア
        mAddressUpdated = false;
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        displayStatusText(getResourceString(R.string.msg_update_address_will_start));
        // Matterデバイス検索処理を開始
        new ChipServiceResolver(this).startResolve();
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
        long deviceId = ChipDeviceIdUtil.getNextAvailableId(getApplicationContext()) - 1;

        // ChipDeviceControllerのアドレス情報を更新
        //   同期型の処理なので、コールバック設定は不要
        try {
            mChipClient.getDeviceController().updateAddress(deviceId, hostAddress, port);
            Log.d(TAG, "Matter device address update completed");
            onUpdateAddressTerminated(true);

        } catch (Exception e) {
            Log.d(TAG, String.format("Matter device address update fail: %s", e));
            onUpdateAddressTerminated(false);
        }
    }

    public void onUpdateAddressTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        if (success) {
            // アドレス更新済みフラグを設定
            mAddressUpdated = true;
            appendStatusText(getResourceString(R.string.msg_update_address_success));
        } else {
            // コマンド実行ボタンを押下不可に設定
            setButtonDoCommandEnabled(false);
            appendStatusText(getResourceString(R.string.msg_update_address_failure));
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
