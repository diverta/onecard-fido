package jp.co.diverta.app.mattercontroller;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

public class ChipServiceDiscoveryListener implements NsdManager.DiscoveryListener
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private ChipServiceResolver resolverRef;

    public ChipServiceDiscoveryListener(ChipServiceResolver csrRef) {
        resolverRef = csrRef;
    }

    @Override
    public void onStartDiscoveryFailed(String serviceType, int errorCode) {
        Log.d(TAG, String.format("Start discovery failed: %d", errorCode));
    }

    @Override
    public void onStopDiscoveryFailed(String serviceType, int errorCode) {
        Log.d(TAG, String.format("Stop discovery failed: %d", errorCode));
    }

    @Override
    public void onDiscoveryStarted(String serviceType) {
        Log.d(TAG, String.format("Discovery started: %s", serviceType));
    }

    @Override
    public void onDiscoveryStopped(String serviceType) {
        Log.d(TAG, String.format("Discovery stopped: %s", serviceType));
    }

    @Override
    public void onServiceFound(NsdServiceInfo serviceInfo) {
        Log.d(TAG, String.format("Service found: %s", serviceInfo.toString()));
        resolverRef.onChipServiceDiscovered(true, serviceInfo);
    }

    @Override
    public void onServiceLost(NsdServiceInfo serviceInfo) {
        Log.d(TAG, String.format("Service lost: %s", serviceInfo.toString()));
        resolverRef.onChipServiceDiscovered(false, serviceInfo);
    }
}
