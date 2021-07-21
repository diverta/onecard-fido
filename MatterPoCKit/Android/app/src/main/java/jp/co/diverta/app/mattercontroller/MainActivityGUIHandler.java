package jp.co.diverta.app.mattercontroller;

import android.graphics.Color;
import android.os.Handler;
import android.os.Message;

public class MainActivityGUIHandler extends Handler
{
    // メッセージID
    public static final int DISPLAY_TEXT = 1;
    public static final int APPEND_TEXT = 2;
    public static final int BUTTONS_ENABLE = 3;
    public static final int BUTTONS_DISABLE = 4;
    public static final int BUTTON_DOCMD_ENABLE = 5;
    public static final int BUTTON_DOCMD_DISABLE = 6;
    public static final int BUTTON_UPADR_ENABLE = 7;
    public static final int BUTTON_UPADR_DISABLE = 8;

    private MainActivity guiRef;

    public MainActivityGUIHandler(MainActivity ma) {
        guiRef = ma;
    }

    public void handleMessage(Message msg) {
        switch (msg.what) {
            case DISPLAY_TEXT:
                // ステータス表示欄に文字列を表示
                guiRef.displayStatusText((String)msg.obj);
                break;
            case APPEND_TEXT:
                // ステータス表示欄に文字列を追加表示
                guiRef.appendStatusText((String)msg.obj);
                break;
            case BUTTONS_ENABLE:
                setButtonsEnabled(true);
                break;
            case BUTTONS_DISABLE:
                setButtonsEnabled(false);
                break;
            case BUTTON_DOCMD_ENABLE:
                setDoCommandButtonEnabled(true);
                break;
            case BUTTON_DOCMD_DISABLE:
                setDoCommandButtonEnabled(false);
                break;
            case BUTTON_UPADR_ENABLE:
                setUpdateAddressButtonEnabled(true);
                break;
            case BUTTON_UPADR_DISABLE:
                setUpdateAddressButtonEnabled(false);
                break;
            default:
                break;
        }
    }

    //
    // 画面オブジェクトを操作するための関数群
    //
    private void setButtonsEnabled(boolean b) {
        // ボタンを押下可能／不可能に変更
        guiRef.buttonPairing.setEnabled(b);
        if (b) {
            guiRef.buttonPairing.setTextColor(Color.BLACK);
        } else {
            guiRef.buttonPairing.setTextColor(Color.GRAY);
        }
        setDoCommandButtonEnabled(b);
    }

    private void setDoCommandButtonEnabled(boolean b) {
        // ボタンを押下可能／不可能に変更
        guiRef.buttonOffCommand.setEnabled(b);
        guiRef.buttonOnCommand.setEnabled(b);
        if (b) {
            guiRef.buttonOffCommand.setTextColor(Color.BLACK);
            guiRef.buttonOnCommand.setTextColor(Color.BLACK);
        } else {
            guiRef.buttonOffCommand.setTextColor(Color.GRAY);
            guiRef.buttonOnCommand.setTextColor(Color.GRAY);
        }
    }

    private void setUpdateAddressButtonEnabled(boolean b) {
        // ボタンを押下可能／不可能に変更
        guiRef.buttonUpdateAddress.setEnabled(b);
        if (b) {
            guiRef.buttonUpdateAddress.setTextColor(Color.BLACK);
        } else {
            guiRef.buttonUpdateAddress.setTextColor(Color.GRAY);
        }
    }
}
