package jp.co.diverta.app.securedongleapp;

import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.widget.Toast;

import jp.co.diverta.app.securedongleapp.ble.BLECentral;
import jp.co.diverta.app.securedongleapp.ble.BLEPeripheral;

public class MainActivityCommand
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private MainActivityGUIHandler handlerRef;
    private BLECentral bleCentral;
    private BLEPeripheral blePeripheral;

    // BLEアドバタイジングが開始されたかどうかを保持
    private boolean mBLEAdvertiseStarted = false;

    // ログ表示用
    private String TAG = getClass().getName();

    public MainActivityCommand(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
        handlerRef = guiRef.guiHandler;
        bleCentral = new BLECentral(this);
        blePeripheral = new BLEPeripheral(this);

        // ペアリング関連ステータスを受信できるようにする
        MainActivityBroadcastReceiver receiver = new MainActivityBroadcastReceiver(this);
        ma.registerBroadcastReceiver(receiver);
    }

    public void startBLEConnection() {
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        displayStatusText(getResourceString(R.string.msg_pairing_will_start));
        // スキャンを開始
        bleCentral.startScan();
    }

    public void cannotStartBLEConnection() {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // BLEが無効の場合
        displayStatusText(getResourceString(R.string.msg_bluetooth_is_turned_off));
    }

    public void onBLEConnectionTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        if (success) {
            appendStatusText(getResourceString(R.string.msg_pairing_success));
        } else {
            appendStatusText(getResourceString(R.string.msg_pairing_failure));
        }
    }

    public void startBLEAdvertise() {
        // アドバタイジング開始済みフラグをクリア
        mBLEAdvertiseStarted = false;
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        // ステータステキストを表示
        displayStatusText(getResourceString(R.string.msg_bleadv_for_auth_will_start));
        // BLEアドバタイズを開始
        blePeripheral.startBLEAdvertise();
    }

    public void stopBLEAdvertise() {
        // アドバタイジング開始済みフラグをクリア
        mBLEAdvertiseStarted = false;
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // ボタンのキャプションを変更
        setButtonAdvertiseChangeCaption(true);
        // ステータステキストを表示
        appendStatusText(getResourceString(R.string.msg_bleadv_for_auth_will_stop));
        // BLEアドバタイズを終了
        blePeripheral.stopBLEAdvertise();
    }

    public void onBLEAdvertiseCallback(boolean success) {
        if (success) {
            // BLE近接認証終了ボタンのみ押下可に変更
            setButtonAdvertiseEnabled(true);
            appendStatusText(getResourceString(R.string.msg_bleadv_for_auth_started));
            // ボタンのキャンプションを変更
            setButtonAdvertiseChangeCaption(false);
            // アドバタイジング開始済みフラグを設定
            mBLEAdvertiseStarted = true;
        } else {
            // 全てのボタンを押下可に変更
            setButtonsEnabled(true);
            appendStatusText(getResourceString(R.string.msg_bleadv_for_auth_start_fail));
        }
    }

    //
    // アドバタイジング自動停止関連
    //

    private Handler mHandler = new Handler();
    private BLEAdvertiseStopperThread mStopperThread = new BLEAdvertiseStopperThread();

    public void onBLEGattServerCallback() {
        // 接続が確立されたら５秒後に切断させるようにする
        mHandler.postDelayed(mStopperThread, 5000);
    }

    private class BLEAdvertiseStopperThread implements Runnable
    {
        @Override
        public void run() {
            // タイムアウトが発生した場合
            Log.d(TAG, "Elapsed 5 seconds after connection");
            // すでにBLEアドバタイジングが停止済みの場合は終了
            if (mBLEAdvertiseStarted == false) {
                Log.d(TAG, "BLE Advertise already stopped");
                return;
            }
            // BLEアドバタイジングを停止
            stopBLEAdvertise();
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

    public void setButtonAdvertiseEnabled(boolean enable) {
        if (enable) {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_ADV_ENABLE);
        } else {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTON_ADV_DISABLE);
        }
    }

    public void setButtonAdvertiseChangeCaption(boolean start) {
        Message msg = Message.obtain(handlerRef, MainActivityGUIHandler.BUTTON_ADV_CHANGE_CAPTION, start);
        handlerRef.sendMessage(msg);
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

    public void popupTinyMessage(int resId) {
        String msg = getResourceString(resId);
        Context context = guiRef.getApplicationContext();
        Toast toast = Toast.makeText(context, msg, Toast.LENGTH_LONG);
        toast.show();
    }

    public void waitMilliSeconds(long ms) {
        try {
            Thread.sleep(ms);
        } catch (Exception ignored) {
        }
    }
}
