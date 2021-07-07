package jp.co.diverta.app.mattercontroller;

import chip.devicecontroller.ChipDeviceController;
import jp.co.diverta.app.mattercontroller.ble.BLECentral;

public class ChipCompletionListener implements ChipDeviceController.CompletionListener
{
    // ログ表示用
    private String TAG = getClass().getName();

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;

    public ChipCompletionListener(BLECentral bc) {
        centralRef = bc;
    }

    @Override
    public void onConnectDeviceComplete() {
    }

    @Override
    public void onSendMessageComplete(String s) {

    }

    @Override
    public void onStatusUpdate(int i) {

    }

    @Override
    public void onPairingComplete(int i) {
        // ペアリング完了時の処理
        boolean success = (i == 0);
        centralRef.onBLEPairingCompleted(success);
    }

    @Override
    public void onPairingDeleted(int i) {

    }

    @Override
    public void onNetworkCommissioningComplete(int i) {

    }

    @Override
    public void onNotifyChipConnectionClosed() {

    }

    @Override
    public void onCloseBleComplete() {

    }

    @Override
    public void onError(Throwable throwable) {

    }

    @Override
    public void onOpCSRGenerationComplete(byte[] bytes) {

    }
}
