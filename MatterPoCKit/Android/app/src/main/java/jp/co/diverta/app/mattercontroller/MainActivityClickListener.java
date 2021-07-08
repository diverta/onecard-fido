package jp.co.diverta.app.mattercontroller;

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
            case R.id.buttonUpdateAddress:
                buttonUpdateAddressClicked();
                break;
            case R.id.buttonDoCommand:
                buttonDoCommandClicked();
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
        // TODO: 仮の仕様です。
        //       MatterデバイスのIPv6アドレス／ポート番号を取得
        commandRef.startUpdateAddress();
    }

    private void buttonDoCommandClicked() {
        String buttonCaption = guiRef.getString(R.string.start_ble_advertisement);
        if (guiRef.buttonDoCommand.getText().equals(buttonCaption)) {
            // TODO: 後日実装

        } else {
            // TODO: 後日実装
        }
    }
}
