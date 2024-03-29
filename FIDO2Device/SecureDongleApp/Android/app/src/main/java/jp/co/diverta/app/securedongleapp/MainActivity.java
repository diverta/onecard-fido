package jp.co.diverta.app.securedongleapp;

import android.Manifest;
import android.bluetooth.BluetoothDevice;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity
{
    // 画面オブジェクト
    public TextView textViewStatus;
    public Button buttonPairing;
    public Button buttonAdvertise;

    // ログ表示用
    private String TAG = getClass().getName();

    // BLEスレッドから画面操作するためのハンドラー
    public MainActivityGUIHandler guiHandler = new MainActivityGUIHandler(this);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "Created");

        // 画面の設定
        setContentView(R.layout.activity_main);
        buttonPairing = findViewById(R.id.buttonPairing);
        buttonAdvertise = findViewById(R.id.buttonAdvertise);
        textViewStatus = findViewById(R.id.textViewStatus);

        // イベントリスナーの設定
        MainActivityClickListener onClickListener = new MainActivityClickListener(this);
        buttonPairing.setOnClickListener(onClickListener);
        buttonAdvertise.setOnClickListener(onClickListener);

        // 位置情報の許可を求める（API 23以降で必須）
        if (checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, 1);
        }
        if (checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.ACCESS_FINE_LOCATION}, 1);
        }
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "Destroyed");
        super.onDestroy();
    }

    //
    // オブジェクトを操作するための関数群
    //

    public void registerBroadcastReceiver(MainActivityBroadcastReceiver receiver) {
        // ペアリング関連ステータスを受信できるようにする
        registerReceiver(receiver, new IntentFilter(BluetoothDevice.ACTION_BOND_STATE_CHANGED));
    }

    public void changeButtonAdvertiseCaption(boolean start) {
        // BLE認証ボタンのキャプションを変更
        if (start) {
            buttonAdvertise.setText(getString(R.string.start_ble_advertisement));
        } else {
            buttonAdvertise.setText(getString(R.string.stop_ble_advertisement));
        }
    }

    public void displayStatusText(String text) {
        // ステータス表示欄に文字列を表示
        textViewStatus.setText(text);
    }

    public void appendStatusText(String text) {
        // ステータス表示欄に文字列を追加表示
        String s = String.format("%s\n%s", textViewStatus.getText(), text);
        displayStatusText(s);
    }
}
