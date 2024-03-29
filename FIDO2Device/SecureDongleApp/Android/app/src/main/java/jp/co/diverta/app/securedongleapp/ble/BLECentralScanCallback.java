package jp.co.diverta.app.securedongleapp.ble;

import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import android.os.ParcelUuid;
import android.util.Log;

import java.util.List;

public class BLECentralScanCallback extends ScanCallback
{
    // UUID関連
    public static final String U2F_SERVICE_UUID = "0000FFFD-0000-1000-8000-00805F9B34FB";

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

        // スキャンされたデバイスの情報を抽出
        List<ParcelUuid> uuids = result.getScanRecord().getServiceUuids();
        if (uuids == null) {
            return;
        }

        // FIDOサービスと同じUUIDであれば、スキャンを停止
        ParcelUuid uuidFIDO = ParcelUuid.fromString(U2F_SERVICE_UUID);
        for (ParcelUuid uuid : uuids) {
            if (uuid.equals(uuidFIDO)) {
                String msg = String.format("FIDO authenticator found: Service UUIDs=%s, Bluetooth address=%s",
                        uuids.toString(),
                        result.getDevice().getAddress());
                Log.d(TAG, msg);
                centralRef.onDeviceScanned(result.getDevice());
                return;
            }
        }
    }

    @Override
    public void onScanFailed(int intErrorCode) {
        super.onScanFailed(intErrorCode);
    }
}
