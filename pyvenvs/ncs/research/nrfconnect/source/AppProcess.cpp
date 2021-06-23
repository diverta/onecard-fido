/* 
 * File:   AppProcess.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 10:38
 */
#include <zephyr.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppProcess);

#include "AppBoltLocker.h"
#include "AppEventHandler.h"
#include "AppDFU.h"
#include "AppLED.h"

//
// for CHIP classes
//   PlatformMgr(), ConnectivityMgr(), ConfigurationMgr()
//
using namespace ::chip;
using namespace ::chip::DeviceLayer;

//
// LED点灯／消灯のための状態管理
//
static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;
static bool sIsFactoryResetTriggered = false;

void AppProcessFactoryResetTriggered(void)
{
    // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
    AppLEDSetBlinkAllLED();
    sIsFactoryResetTriggered = true;
    LOG_INF("Factory Reset Triggered. Release button within 3 seconds to cancel.");
}

void AppProcessFactoryResetCancelled(void)
{
    // Set lock status LED back to show state of lock.
    AppLEDSetToggleLED2(AppBoltLockerIsLocked());
    sIsFactoryResetTriggered = false;
    LOG_INF("Factory Reset has been Canceled");
}

static void updateLEDStatus(void)
{
    // Update the status LED if factory reset has not been initiated.
    //
    // If system has "full connectivity", keep the LED On constantly.
    //
    // If thread and service provisioned, but not attached to the thread network yet OR no
    // connectivity to the service OR subscriptions are not fully established
    // THEN blink the LED Off for a short period of time.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
    // rate of 100ms.
    //
    // Otherwise, blink the LED ON for a very short time.
    if (sIsFactoryResetTriggered == false) {
        if (sHaveServiceConnectivity) {
            // サービス実行中
            AppLEDKeepOnLED1();

        } else if (sIsThreadProvisioned && sIsThreadEnabled) {
            // Thread使用可能状態
            AppLEDSetLongBlinkLED1();

        } else if (sHaveBLEConnections) {
            // BLE接続状態
            AppLEDSetHalfBlinkLED1();

        } else {
            // アイドル状態
            AppLEDSetShortBlinkLED1();
        }
    }

    AppLEDAnimate();
}

static void collectStatesFromCHIPStack(void)
{
    // Collect connectivity and configuration state from the CHIP stack.  Because the
    // CHIP event loop is being run in a separate task, the stack must be locked
    // while these values are queried.  However we use a non-blocking lock request
    // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
    // task is busy (e.g. with a long crypto operation).
    if (PlatformMgr().TryLockChipStack()) {
        sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
        sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
        sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
        PlatformMgr().UnlockChipStack();
    }
}

//
// サービスパブリッシュ関連
//
#include "Service.h"

void publishService(void)
{
    // ５秒以上の間隔でサービスをパブリッシュ
    static uint32_t kPublishServicePeriodUs = 5000000;
    static uint64_t mLastPublishServiceTimeUS = 0;

    uint64_t nowUS = chip::System::Platform::Layer::GetClock_Monotonic();
    uint64_t nextChangeTimeUS = mLastPublishServiceTimeUS + kPublishServicePeriodUs;

    if (nowUS > nextChangeTimeUS) {
        PublishService();
        mLastPublishServiceTimeUS = nowUS;
    }
}

//
// 初期化処理
//
static bool applicationProcessInit()
{
    // LED初期化処理
    AppLEDInit();

    // ボタンイベント用の初期化処理
    if (AppEventHandlerInit() == false) {
        return false;
    }

    // Confirm firmware image
    AppDFUConfirmFirmwareImage();

    // Bolt lock manager
    AppBoltLockerInitialize();

    // Lock状態をLEDに反映
    AppLEDSetToggleLED2(AppBoltLockerIsLocked());

    // Init ZCL Data Model and start server
    InitServer();
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    return true;
}

//
// アプリケーション処理本体
//
int AppProcessMain(void)
{
    // アプリケーション初期化処理
    if (applicationProcessInit() == false) {
        return 1;
    }

    // アプリケーション処理を実行
    while (AppEventHandlerDispatch() == 0) {
        // CHIPスタックの状態を取得し、LEDに反映
        collectStatesFromCHIPStack();
        updateLEDStatus();
        publishService();
    }

    return 0;
}
