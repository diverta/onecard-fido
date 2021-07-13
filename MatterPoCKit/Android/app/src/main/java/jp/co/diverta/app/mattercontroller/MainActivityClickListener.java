package jp.co.diverta.app.mattercontroller;

import android.view.View;

public class MainActivityClickListener implements View.OnClickListener
{
    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    public MainActivityClickListener(MainActivityCommand mac) {
        // コマンドクラスの参照を保持
        commandRef = mac;
    }

    @Override
    public void onClick(View v) {
        switch (v.getId()) {
            case R.id.buttonPairing:
                buttonPairingClicked();
                break;
            case R.id.buttonUpdateAddress:
                buttonUpdateAddressClicked();
                break;
            case R.id.buttonOffCommand:
                buttonOffCommandClicked();
                break;
            case R.id.buttonOnCommand:
                buttonOnCommandClicked();
                break;
            default:
                break;
        }
    }

    private void buttonPairingClicked() {
        // BLEペアリング-->コミッショニングを開始
        commandRef.startBLEConnection();
    }

    private void buttonUpdateAddressClicked() {
        // MatterデバイスのIPv6アドレス／ポート番号を取得し、
        // フレームワークに設定
        commandRef.startUpdateAddress();
    }

    private void buttonOffCommandClicked() {
        // Offコマンドを実行
        commandRef.performOffCommand();
    }

    private void buttonOnCommandClicked() {
        // Onコマンドを実行
        commandRef.performOnCommand();
    }
}
