package jp.co.diverta.app.mattercontroller;

import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

public class ChipServiceResolverListener implements NsdManager.ResolveListener
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private ChipServiceResolver resolverRef;

    public ChipServiceResolverListener(ChipServiceResolver csrRef) {
        resolverRef = csrRef;
    }

    @Override
    public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
        // 処理クラスに制御を戻す
        Log.d(TAG, String.format("Resolve failed: %d", errorCode));
        resolverRef.onChipServiceResolved(false, serviceInfo);
    }

    @Override
    public void onServiceResolved(NsdServiceInfo serviceInfo) {
        // 処理クラスに制御を戻す
        resolverRef.onChipServiceResolved(true, serviceInfo);
    }
}
