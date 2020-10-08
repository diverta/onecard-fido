package jp.co.diverta.app.securedongleapp;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity
{
    // 画面オブジェクト
    public TextView textViewStatus;
    public Button buttonPairing;
    public Button buttonAdvertise;

    // ログ表示用
    private String TAG;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        TAG = getString(R.string.app_name);
        Log.d(TAG, "MainActivity created");

        // 画面の設定
        setContentView(R.layout.activity_main);
        buttonPairing = findViewById(R.id.buttonPairing);
        buttonAdvertise = findViewById(R.id.buttonAdvertise);
        textViewStatus = findViewById(R.id.textViewStatus);

        // イベントリスナーの設定
        MainActivityClickListener onClickListener = new MainActivityClickListener(this);
        buttonPairing.setOnClickListener(onClickListener);
        buttonAdvertise.setOnClickListener(onClickListener);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "MainActivity destroyed");
        super.onDestroy();
    }

    //
    // オブジェクトを操作するための関数群
    //

    public void changeButtonAdvertiseCaption(boolean start) {
        if (start) {
            buttonAdvertise.setText(getString(R.string.start_ble_advertisement));
        } else {
            buttonAdvertise.setText(getString(R.string.stop_ble_advertisement));
        }
    }

    public void displayStatusText(String text) {
        textViewStatus.setText(text);
    }

    public void appendStatusText(String text) {
        String s = String.format("%s\n%s", textViewStatus.getText(), text);
        displayStatusText(s);
    }
}
