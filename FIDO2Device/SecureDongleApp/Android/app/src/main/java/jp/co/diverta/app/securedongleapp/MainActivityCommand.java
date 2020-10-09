package jp.co.diverta.app.securedongleapp;

import android.bluetooth.BluetoothManager;
import android.content.Context;

import jp.co.diverta.app.securedongleapp.ble.BLECentral;

public class MainActivityCommand
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private BLECentral bleCentral;

    // ログ表示用
    private String TAG = getClass().getName();

    public MainActivityCommand(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
        bleCentral = new BLECentral(this);
    }

    public void startBLEConnection() {
        // スキャンを開始
        bleCentral.startScan();
    }

    public void startBLEAdvertise() {
        // TODO: BLEアドバタイズを開始
    }

    public void stopBLEAdvertise() {
        // TODO: BLEアドバタイズを終了
    }

    //
    // MainActivityにアクセスするための関数群
    //

    public void displayStatusText(String s) {
        // ステータス表示欄に文字列を表示
        MainActivityGUIHandler handler = guiRef.guiHandler;
        handler.setStatusText(s);
        handler.sendEmptyMessage(MainActivityGUIHandler.DISPLAY_TEXT);
    }

    public void appendStatusText(String s) {
        // ステータス表示欄に文字列を追加表示
        MainActivityGUIHandler handler = guiRef.guiHandler;
        handler.setStatusText(s);
        handler.sendEmptyMessage(MainActivityGUIHandler.APPEND_TEXT);
    }

    public BluetoothManager getBluetoothManager() {
        return (BluetoothManager)guiRef.getSystemService(Context.BLUETOOTH_SERVICE);
    }

    public Context getApplicationContext() {
        return guiRef.getApplicationContext();
    }
}
