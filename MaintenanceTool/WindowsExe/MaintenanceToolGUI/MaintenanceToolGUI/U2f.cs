using MaintenanceToolCommon;
using System;

namespace MaintenanceToolGUI
{
    class U2f
    {
        // トランスポート種別を保持
        private byte transportType;

        // トランスポート別処理の参照を保持
        private HIDMain hidMain;
        private BLEMain bleMain;

        // メイン画面の参照を保持
        private MainForm mainForm;

        // 実行機能を保持
        public enum RequestType
        {
            None = 0,
            TestRegister,
            TestAuthenticateCheck,
            TestAuthenticate,
        };
        private RequestType requestType;

        public U2f(MainForm m, byte transportType_)
        {
            mainForm = m;
            transportType = transportType_;
        }

        public void SetHidMain(HIDMain p)
        {
            hidMain = p;
        }

        public void SetBleMain(BLEMain a)
        {
            bleMain = a;
        }

        public void SetRequestType(RequestType t)
        {
            requestType = t;
        }
    }
}
