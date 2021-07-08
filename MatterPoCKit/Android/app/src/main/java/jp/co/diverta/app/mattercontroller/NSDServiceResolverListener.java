package jp.co.diverta.app.mattercontroller;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

import chip.devicecontroller.ChipDeviceController;

public class NSDServiceResolverListener implements NsdManager.ResolveListener
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;
    private long mDeviceId;

    public NSDServiceResolverListener(MainActivityCommand mac, long deviceId) {
        commandRef = mac;
        mDeviceId = deviceId;
    }

    @Override
    public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
        Log.d(TAG, String.format("Address resolution failed: %d", errorCode));
        commandRef.onUpdateAddressTerminated(false);
    }

    @Override
    public void onServiceResolved(NsdServiceInfo serviceInfo) {
        // アドレスを取得
        String hostAddress = serviceInfo.getHost().getHostAddress();
        if (hostAddress == null) {
            hostAddress = "";
        }

        // ポート番号を取得
        int port = serviceInfo.getPort();
        if (hostAddress.equals("") || port == 0) {
            return;
        }

        try {
            // デバイスIDで検索実行
            ChipDeviceController deviceController = new ChipDeviceController();
            deviceController.updateAddress(mDeviceId, hostAddress, port);
            Log.d(TAG, String.format("deviceController.updateAddress called: %s %d", hostAddress, port));

            // TODO: 仮の仕様です。
            commandRef.onUpdateAddressTerminated(true);

        } catch (Exception e) {
            Log.e(TAG, String.format("Address update fail", e));
            commandRef.onUpdateAddressTerminated(false);
        }
    }
}
