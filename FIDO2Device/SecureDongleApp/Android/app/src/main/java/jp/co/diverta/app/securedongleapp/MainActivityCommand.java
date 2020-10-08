package jp.co.diverta.app.securedongleapp;

public class MainActivityCommand
{
    // オブジェクトの参照を保持
    private MainActivity guiRef;

    // ログ表示用
    private String TAG = getClass().getName();

    public MainActivityCommand(MainActivity ma) {
        // 画面オブジェクトの参照を保持
        guiRef = ma;
    }

    public void startBLEConnection() {
        // TODO: スキャンを開始
    }

    public void startBLEAdvertise() {
        // TODO: BLEアドバタイズを開始
    }

    public void stopBLEAdvertise() {
        // TODO: BLEアドバタイズを終了
    }

    //
    // MainActivityにアクセスするための関数群
    //

    public void displayStatusText(String s) {
        // ステータス表示欄に文字列を表示
        MainActivityGUIHandler handler = guiRef.guiHandler;
        handler.setStatusText(s);
        handler.sendEmptyMessage(MainActivityGUIHandler.DISPLAY_TEXT);
    }

    public void appendStatusText(String s) {
        // ステータス表示欄に文字列を追加表示
        MainActivityGUIHandler handler = guiRef.guiHandler;
        handler.setStatusText(s);
        handler.sendEmptyMessage(MainActivityGUIHandler.APPEND_TEXT);
    }
}
