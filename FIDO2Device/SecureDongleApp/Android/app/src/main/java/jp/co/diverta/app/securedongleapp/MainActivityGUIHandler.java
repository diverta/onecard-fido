package jp.co.diverta.app.securedongleapp;

import android.os.Handler;
import android.os.Message;

public class MainActivityGUIHandler extends Handler
{
    // メッセージID
    public static final int DISPLAY_TEXT = 1;
    public static final int APPEND_TEXT = 2;

    private MainActivity guiRef;
    private String statusText;

    public MainActivityGUIHandler(MainActivity ma) {
        guiRef = ma;
    }

    public void handleMessage(Message msg) {
        switch (msg.what) {
            case DISPLAY_TEXT:
                // ステータス表示欄に文字列を表示
                guiRef.displayStatusText(statusText);
                break;
            case APPEND_TEXT:
                // ステータス表示欄に文字列を追加表示
                guiRef.appendStatusText(statusText);
                break;
            default:
                break;
        }
    }

    //
    // 画面オブジェクトを操作するための関数群
    //

    public void setStatusText(String s) {
        // ステータス欄に表示する文字列を設定
        statusText = s;
    }
}
