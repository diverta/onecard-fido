package jp.co.diverta.app.mattercontroller;

import chip.devicecontroller.ChipDeviceController;

public class ChipClient
{
    // ChipDeviceControllerの参照を保持
    private ChipDeviceController deviceController = null;

    // コマンドクラスの参照を保持
    private MainActivityCommand commandRef;

    public ChipClient(MainActivityCommand mac) {
        commandRef = mac;
    }

    public ChipDeviceController getDeviceController() {
        if (deviceController == null) {
            deviceController = new ChipDeviceController();
        }
        return deviceController;
    }
}
