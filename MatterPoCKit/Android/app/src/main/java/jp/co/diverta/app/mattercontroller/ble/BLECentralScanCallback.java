package jp.co.diverta.app.mattercontroller.ble;

import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.util.Log;

public class BLECentralScanCallback extends ScanCallback
{
    // ログ表示用
    private String TAG = getClass().getName();

    // セントラルマネージャーの参照を保持
    private BLECentral centralRef;

    public BLECentralScanCallback(BLECentral bc) {
        centralRef = bc;
    }

    @Override
    public void onScanResult(int callbackType, ScanResult result) {
        super.onScanResult(callbackType, result);

        String mesg = String.format("Bluetooth Device Scanned: address=%s",
                result.getDevice().getAddress());
        Log.d(TAG, mesg);
        centralRef.onDeviceScanned(result.getDevice());
    }

    @Override
    public void onScanFailed(int intErrorCode) {
        super.onScanFailed(intErrorCode);
    }
}
