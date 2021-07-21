package jp.co.diverta.app.mattercontroller.chip;

import android.util.Log;

import chip.devicecontroller.ChipClusters.DefaultClusterCallback;
import jp.co.diverta.app.mattercontroller.MainActivityCommand;

public class ClusterCallback implements DefaultClusterCallback
{
    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    public ClusterCallback(MainActivityCommand mac) {
        commandRef = mac;
    }

    @Override
    public void onSuccess() {
        commandRef.onOnOffCommandTerminated(true);
    }

    @Override
    public void onError(Exception e) {
        commandRef.onOnOffCommandTerminated(false);
    }
}
