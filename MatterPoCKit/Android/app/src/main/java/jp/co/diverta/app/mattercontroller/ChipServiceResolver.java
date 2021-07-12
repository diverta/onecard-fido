package jp.co.diverta.app.mattercontroller;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

import static android.net.nsd.NsdManager.PROTOCOL_DNS_SD;

public class ChipServiceResolver
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    // NSD Managerの参照を保持
    private NsdManager mNsdManager = null;

    // ディスカバリー開始が行われたかどうかを保持
    private boolean mDiscoveryStarted = false;

    public ChipServiceResolver(MainActivityCommand mac) {
        commandRef = mac;
    }

    public void startResolve() {
        // アプリケーション共有情報を取得
        Context context = commandRef.getApplicationContext();

        // NSD Managerにデバイス検索（サービス解決）を依頼
        Object systemService = context.getSystemService(Context.NSD_SERVICE);
        if (systemService == null) {
            // NSD Serviceの参照取得が失敗した場合は、
            // コマンドクラスに制御を戻す
            Log.e(TAG, "NSD service manager cannot get");
            commandRef.onChipServiceResolved(false);
            return;
        }

        // NSD Managerにサービス検索を依頼
        mNsdManager = (NsdManager)systemService;
        String serviceType = "_matter._tcp";
        ChipServiceDiscoveryListener discoveryListener = new ChipServiceDiscoveryListener(this);
        mNsdManager.discoverServices(serviceType, PROTOCOL_DNS_SD, discoveryListener);
        mDiscoveryStarted = true;
    }

    public void onChipServiceDiscovered(boolean success, NsdServiceInfo serviceInfo) {
        // ユーザーによりディスカバリーが開始されていない場合は終了
        if (mDiscoveryStarted == false) {
            return;
        }
        mDiscoveryStarted = false;

        // ディスカバリー内容が不正の場合は終了
        if (success == false || serviceInfo == null) {
            return;
        }
        Log.d(TAG, String.format("Discovered SRP instance=%s, name=%s", serviceInfo.getServiceName(), serviceInfo.getServiceType()));

        // このアプリで保持しているService Nameと異なる場合は終了
        Context context = commandRef.getApplicationContext();
        long fabricId = 5544332211L;
        String serviceName = ChipDeviceIdUtil.getServiceNameFromAvailableId(context, fabricId);
        if (serviceInfo.getServiceName().equals(serviceName) == false) {
            Log.d(TAG, String.format("Discovered SRP instance not commissioned by this APP(%s)", serviceName));
            return;
        }

        // NSD Managerにデバイス検索を依頼
        ChipServiceResolverListener resolverListener = new ChipServiceResolverListener(this);
        mNsdManager.resolveService(serviceInfo, resolverListener);
    }

    public void onChipServiceResolved(boolean success, NsdServiceInfo serviceInfo) {
        if (success == false || serviceInfo == null) {
            return;
        }

        // デバイスのIPv6アドレス／ポート番号を取得
        String hostAddress = serviceInfo.getHost().getHostAddress();
        int port = serviceInfo.getPort();
        Log.d(TAG, String.format("Resolved device: Host=%s Port=%d", hostAddress, port));

        // コマンドクラスに制御を戻す
        commandRef.onChipServiceResolved(true);
    }
}
