package jp.co.diverta.app.mattercontroller.chip;

import android.util.Log;

import chip.devicecontroller.ChipClusters.DefaultClusterCallback;
import jp.co.diverta.app.mattercontroller.MainActivityCommand;

public class ClusterCallback implements DefaultClusterCallback
{
    // ログ表示用
    private String TAG = getClass().getName();

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    public ClusterCallback(MainActivityCommand mac) {
        commandRef = mac;
    }

    @Override
    public void onSuccess() {
        Log.i(TAG, "Command success");
        commandRef.onOnOffCommandTerminated(true);
    }

    @Override
    public void onError(Exception e) {
        Log.e(TAG, "Command failed", e);
        commandRef.onOnOffCommandTerminated(false);
    }
}
