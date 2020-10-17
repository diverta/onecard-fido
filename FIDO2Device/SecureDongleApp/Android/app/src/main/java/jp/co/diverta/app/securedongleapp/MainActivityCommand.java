package jp.co.diverta.app.securedongleapp;

import android.bluetooth.BluetoothManager;
import android.content.Context;
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
        displayStatusText("ペアリングを開始します。");
        // スキャンを開始
        bleCentral.startScan();
    }

    public void cannotStartBLEConnection() {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        // BLEが無効の場合
        displayStatusText("Bluetoothがオフになっています。Bluetoothをオンにしてください。");
    }

    public void onBLEConnectionTerminated(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        if (success) {
            appendStatusText("ペアリングが成功しました。");
        } else {
            appendStatusText("ペアリングが失敗しました。");
        }
    }

    public void startBLEAdvertise() {
        // ボタンを押下不可に変更
        setButtonsEnabled(false);
        // ステータステキストを表示
        displayStatusText("BLE認証を開始します。");
        // BLEアドバタイズを開始
        blePeripheral.startBLEAdvertise();
    }

    public void stopBLEAdvertise() {
        // ステータステキストを表示
        appendStatusText("BLE認証を終了します。");
        // BLEアドバタイズを終了
        blePeripheral.stopBLEAdvertise();
    }

    public void onBLEAdvertiseCallback(boolean success) {
        // ボタンを押下可に変更
        setButtonsEnabled(true);
        if (success) {
            appendStatusText("アドバタイジングが開始されました。");
        } else {
            appendStatusText("アドバタイジングを開始できませんでした。");
        }
    }

    //
    // MainActivityにアクセスするための関数群
    //

    public void displayStatusText(String s) {
        // ステータス表示欄に文字列を表示
        handlerRef.setStatusText(s);
        handlerRef.sendEmptyMessage(MainActivityGUIHandler.DISPLAY_TEXT);
    }

    public void appendStatusText(String s) {
        // ステータス表示欄に文字列を追加表示
        handlerRef.setStatusText(s);
        handlerRef.sendEmptyMessage(MainActivityGUIHandler.APPEND_TEXT);
    }

    public void setButtonsEnabled(boolean enable) {
        if (enable) {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTONS_ENABLE);
        } else {
            handlerRef.sendEmptyMessage(MainActivityGUIHandler.BUTTONS_DISABLE);
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

    public void popupTinyMessage(String msg) {
        Context context = guiRef.getApplicationContext();
        Toast toast = Toast.makeText(context, msg, Toast.LENGTH_SHORT);
        toast.show();
    }

    public void waitMilliSeconds(long ms) {
        try {
            Thread.sleep(ms);
        } catch (Exception ignored) {
        }
    }
}
