package jp.co.diverta.app.securedongleapp;

import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class MainActivityBroadcastReceiver extends BroadcastReceiver
{
    // オブジェクトの参照を保持
    private MainActivityCommand commandRef;

    // ログ表示用
    private String TAG = getClass().getName();

    // ボンディング処理中ステータスを保持
    private boolean mBondingInProgress = false;

    public MainActivityBroadcastReceiver(MainActivityCommand mac) {
        // 画面オブジェクトの参照を保持
        commandRef = mac;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        // ペアリング関連でない場合は終了
        String action = intent.getAction();
        if (action.equals(BluetoothDevice.ACTION_BOND_STATE_CHANGED) == false) {
            Log.d(TAG, "BroadcastReceiver action: " + action);
            return;
        }

        // ペアリング関連ステータスを取得
        int bondState = intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE, BluetoothDevice.ERROR);
        switch (bondState) {
            case BluetoothDevice.BOND_NONE:
                onBondingNotDone();
                break;
            case BluetoothDevice.BOND_BONDING:
                onBondingInProgress();
                break;
            case BluetoothDevice.BOND_BONDED:
                onBondingDone();
                break;
            default:
                Log.d(TAG, "Bond state: unknown " + bondState);
                break;
        }
    }

    private void onBondingInProgress() {
        Log.d(TAG, "Bond state: BOND_BONDING");
        mBondingInProgress = true;
    }

    private void onBondingNotDone() {
        // ペアリングが行われなかった場合
        Log.d(TAG, "Bond state: BOND_NONE");
        if (mBondingInProgress) {
            // コマンドクラスに制御を戻す
            commandRef.onBLEConnectionTerminated(false);
            mBondingInProgress = false;
        }
    }

    private void onBondingDone() {
        // ペアリングが完了した場合
        Log.d(TAG, "Bond state: BOND_BONDED");
        if (mBondingInProgress) {
            // コマンドクラスに制御を戻す
            commandRef.onBLEConnectionTerminated(true);
            mBondingInProgress = false;
        }
    }
}
