package jp.co.diverta.app.mattercontroller;

import android.Manifest;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import chip.devicecontroller.ChipDeviceController;
import chip.devicecontroller.PreferencesKeyValueStoreManager;

public class MainActivity extends AppCompatActivity
{
    // 画面オブジェクト
    public TextView textViewStatus;
    public Button buttonPairing;
    public Button buttonUpdateAddress;
    public Button buttonOffCommand;
    public Button buttonOnCommand;

    // ログ表示用
    private String TAG = getClass().getName();

    // 別スレッドから画面操作するためのハンドラー
    public MainActivityGUIHandler guiHandler = new MainActivityGUIHandler(this);

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "Created");

        // 画面の設定
        setContentView(R.layout.activity_main);
        buttonPairing = findViewById(R.id.buttonPairing);
        buttonUpdateAddress = findViewById(R.id.buttonUpdateAddress);
        buttonOffCommand = findViewById(R.id.buttonOffCommand);
        buttonOnCommand = findViewById(R.id.buttonOnCommand);
        textViewStatus = findViewById(R.id.textViewStatus);

        if (savedInstanceState == null) {
            // 共有情報の初期化
            ChipDeviceController.setKeyValueStoreManager(new PreferencesKeyValueStoreManager(getApplicationContext()));
        }

        // コマンドクラスの生成
        MainActivityCommand mac = new MainActivityCommand(this);

        // イベントリスナーの設定
        MainActivityClickListener onClickListener = new MainActivityClickListener(mac);
        buttonPairing.setOnClickListener(onClickListener);
        buttonUpdateAddress.setOnClickListener(onClickListener);
        buttonOffCommand.setOnClickListener(onClickListener);
        buttonOnCommand.setOnClickListener(onClickListener);

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
