package jp.co.diverta.app.mattercontroller;

import android.content.Context;
import android.content.SharedPreferences;

public class ChipDeviceIdUtil
{
    private static final String PREFERENCE_FILE_KEY = "com.google.chip.chiptool.PREFERENCE_FILE_KEY";
    private static final String DEVICE_ID_PREFS_KEY = "device_id";
    private static long DEFAULT_DEVICE_ID = 1L;

    public ChipDeviceIdUtil() {
    }

    private static SharedPreferences getPrefs(Context context) {
        return context.getSharedPreferences(PREFERENCE_FILE_KEY, Context.MODE_PRIVATE);
    }

    public static long getNextAvailableId(Context context) {
        SharedPreferences prefs = getPrefs(context);
        if (prefs.contains(DEVICE_ID_PREFS_KEY)) {
            // 存在する場合
            return prefs.getLong(DEVICE_ID_PREFS_KEY, DEFAULT_DEVICE_ID);
        } else {
            // 存在しない場合はデフォルトのデバイスIDを書出
            prefs.edit().putLong(DEVICE_ID_PREFS_KEY, DEFAULT_DEVICE_ID).apply();
            return DEFAULT_DEVICE_ID;
        }
    }

    public static void setNextAvailableId(Context context, long newId) {
        getPrefs(context).edit().putLong(DEVICE_ID_PREFS_KEY, newId).apply();
    }

    public static String getServiceNameFromAvailableId(Context context, long fabricId) {
        // このアプリで保持しているDevice IDと
        // fabric IDから、
        // "000000014A77CBB3-0000000000000005"形式の
        // Service Nameを生成
        long deviceId = getNextAvailableId(context) - 1;
        String serviceName = String.format("%016X-%016X", fabricId, deviceId);
        return serviceName;
    }
}
