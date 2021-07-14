package jp.co.diverta.app.mattercontroller;

import chip.devicecontroller.ChipDeviceController;
import jp.co.diverta.app.mattercontroller.ble.BLECentral;

public class ChipCompletionListener implements ChipDeviceController.CompletionListener
{
    // ログ表示用
    private String TAG = getClass().getName();

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;

    private boolean mCommissioningCompleted = false;

    public ChipCompletionListener(BLECentral bc) {
        mCommissioningCompleted = false;
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
        if (i == 0) {
            // ペアリング完了時の処理
            centralRef.onBLEPairingCompleted(true);
            return;
        }
        // すでに一連の処理が完了している場合は無視
        if (mCommissioningCompleted == false) {
            centralRef.onBLEPairingCompleted(false);
            mCommissioningCompleted = true;
        }
    }

    @Override
    public void onPairingDeleted(int i) {

    }

    @Override
    public void onNetworkCommissioningComplete(int i) {
        if (i == 0) {
            // コミッショニング完了時の処理
            centralRef.onCommissioningComplete(true);
            mCommissioningCompleted = true;
            return;
        }
        // すでに一連の処理が完了している場合は無視
        if (mCommissioningCompleted == false) {
            centralRef.onCommissioningComplete(false);
            mCommissioningCompleted = true;
        }
    }

    @Override
    public void onNotifyChipConnectionClosed() {

    }

    @Override
    public void onCloseBleComplete() {

    }

    @Override
    public void onError(Throwable throwable) {
        // すでに一連の処理が完了している場合は無視
        if (mCommissioningCompleted == false) {
            centralRef.onCommissioningComplete(false);
            mCommissioningCompleted = true;
        }
    }

    @Override
    public void onOpCSRGenerationComplete(byte[] bytes) {

    }
}
