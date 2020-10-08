package jp.co.diverta.app.securedongleapp;

import android.view.View;

public class MainActivityClickListener implements View.OnClickListener
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;
    private MainActivityCommand commandRef;

    // ボタンキャプション切り替え用のフラグ
    private boolean flag = false;
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
            // ステータステキストを表示
            String msg = getResourceString(R.string.status_text_ble_adv_start);
            guiRef.displayStatusText(msg);
            // ボタンキャプションを変更
            guiRef.changeButtonAdvertiseCaption(false);
            advertising = true;
            // アドバタイズ開始
            commandRef.startBLEAdvertise();

        } else {
            // ステータステキストに追加表示
            String msg = getResourceString(R.string.status_text_ble_adv_stop);
            guiRef.appendStatusText(msg);
            // ボタンキャプションを変更
            guiRef.changeButtonAdvertiseCaption(true);
            advertising = false;
            // アドバタイズ停止
            commandRef.stopBLEAdvertise();
        }
    }

    //
    // MainActivityにアクセスするための関数群
    //

    private String getResourceString(int resId) {
        return guiRef.getString(resId);
    }
}
