package jp.co.diverta.app.mattercontroller;

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.os.Handler;
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

    // ディスカバリーが開始されたかどうかを保持
    private boolean mDiscoveryStarted = false;

    // ディスカバリー処理のコールバックを保持
    private ChipServiceDiscoveryListener mDiscoveryListener;
    private ChipServiceResolverListener mResolverListener;

    public ChipServiceResolver(MainActivityCommand mac) {
        commandRef = mac;
        mDiscoveryListener = new ChipServiceDiscoveryListener(this);
        mResolverListener = new ChipServiceResolverListener(this);
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
            commandRef.onChipServiceResolved(false, null);
            return;
        }

        // NSD Managerにサービス検索を依頼
        mNsdManager = (NsdManager)systemService;
        String serviceType = "_matter._tcp";
        mNsdManager.discoverServices(serviceType, PROTOCOL_DNS_SD, mDiscoveryListener);
        mDiscoveryStarted = true;

        // 操作タイムアウト監視を開始
        startOperationTimeout();
    }

    public void onChipServiceDiscovered(boolean success, NsdServiceInfo serviceInfo) {
        // ユーザーによりディスカバリーが開始されていない場合は終了
        if (mDiscoveryStarted == false) {
            return;
        }

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

        // ディスカバー処理を停止
        stopDiscover();

        // NSD Managerにデバイス検索を依頼
        mNsdManager.resolveService(serviceInfo, mResolverListener);
    }

    private void stopDiscover() {
        // ディスカバリー開始済みフラグをクリア
        mDiscoveryStarted = false;

        // 操作タイムアウトの監視を停止
        cancelOperationTimeout();

        // ディスカバー処理を停止
        mNsdManager.stopServiceDiscovery(mDiscoveryListener);
    }


    public void onChipServiceResolved(boolean success, NsdServiceInfo serviceInfo) {
        if (success == false || serviceInfo == null) {
            return;
        }

        // コマンドクラスに制御を戻す
        commandRef.onChipServiceResolved(true, serviceInfo);
    }

    //
    // 操作タイムアウト監視関連
    //
    public void startOperationTimeout() {
        // 操作指示から５秒後にタイムアウトさせるようにする
        mOperationTimeoutThread = new ChipServiceResolver.OperationTimeoutThread();
        mOperationTimeoutHandler.postDelayed(mOperationTimeoutThread, 5000);
    }

    public void cancelOperationTimeout() {
        // タイムアウト監視を停止
        if (mOperationTimeoutThread != null) {
            mOperationTimeoutHandler.removeCallbacks(mOperationTimeoutThread);
            mOperationTimeoutThread = null;
        }
    }

    private class OperationTimeoutThread implements Runnable
    {
        @Override
        public void run() {
            // すでに停止済みの場合は終了
            if (mDiscoveryStarted == false) {
                return;
            }
            // タイムアウトが発生した場合は、操作を停止
            commandRef.appendStatusText(commandRef.getResourceString(R.string.msg_operation_timeout));
            Log.d(TAG, "SRP service discover operation timed out");
            stopDiscover();

            // コマンドクラスに制御を戻す
            commandRef.onChipServiceResolved(false, null);
        }
    }

    // 操作タイムアウト監視で使用するオブジェクト
    private Handler mOperationTimeoutHandler = new Handler();
    private ChipServiceResolver.OperationTimeoutThread mOperationTimeoutThread = null;
}
