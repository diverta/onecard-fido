package jp.co.diverta.app.securedongleapp;

import android.view.View;

public class MainActivityClickListener implements View.OnClickListener
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private MainActivityCommand commandRef;

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
        String buttonStartCaption = guiRef.getString(R.string.start_ble_advertisement);
        if (guiRef.buttonAdvertise.getText().equals(buttonStartCaption)) {
            // BLE近接認証実行ボタンの場合
            // アドバタイズ開始
            commandRef.startBLEAdvertise();

        } else {
            // BLE近接認証終了ボタンの場合
            // ボタンキャプションを変更
            guiRef.changeButtonAdvertiseCaption(true);
            // アドバタイズ停止
            commandRef.stopBLEAdvertise();
        }
    }
}
