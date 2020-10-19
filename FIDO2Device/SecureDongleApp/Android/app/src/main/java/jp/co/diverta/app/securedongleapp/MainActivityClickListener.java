package jp.co.diverta.app.securedongleapp;

import android.view.View;

public class MainActivityClickListener implements View.OnClickListener
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private MainActivityCommand commandRef;

    // ボタンキャプション切り替え用のフラグ
    private boolean advertising = false;

    public MainActivityClickListener(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
        commandRef = new MainActivityCommand(ma);
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.buttonPairing:
                buttonPairingClicked();
                break;
            case R.id.buttonAdvertise:
                buttonAdvertiseClicked();
                break;
            default:
                break;
        }
    }

    private void buttonPairingClicked() {
        // BLE接続を開始
        commandRef.startBLEConnection();
    }

    private void buttonAdvertiseClicked() {
        if (advertising == false) {
            // ボタンキャプションを変更
            guiRef.changeButtonAdvertiseCaption(false);
            advertising = true;
            // アドバタイズ開始
            commandRef.startBLEAdvertise();

        } else {
            // ボタンキャプションを変更
            guiRef.changeButtonAdvertiseCaption(true);
            advertising = false;
            // アドバタイズ停止
            commandRef.stopBLEAdvertise();
        }
    }
}
