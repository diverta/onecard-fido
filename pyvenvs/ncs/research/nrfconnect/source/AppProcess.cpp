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
#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppProcess);

#include "AppBoltLocker.h"
#include "AppEventHandler.h"
#include "AppDFU.h"

//
// for CHIP classes
//   PlatformMgr(), ConnectivityMgr(), ConfigurationMgr()
//
using namespace ::chip;
using namespace ::chip::DeviceLayer;

//
// LED関連
//
#include "LEDWidget.h"

// LEDのインスタンス
static LEDWidget sLED_1;
static LEDWidget sLED_2;
static LEDWidget sLED_3;
static LEDWidget sLED_4;

void AppLEDInit(void)
{
    // Initialize LEDs
    LEDWidget::InitGpio();
    sLED_1.Init(DK_LED1);
    sLED_2.Init(DK_LED2);
    sLED_3.Init(DK_LED3);
    sLED_4.Init(DK_LED4);
}

void AppLEDAnimate(void)
{
    // LED点滅処理を実行
    sLED_1.Animate();
    sLED_2.Animate();
    sLED_3.Animate();
    sLED_4.Animate();
}

void AppLEDKeepOnLED1(void)
{
    sLED_1.Set(true);
}

void AppLEDSetLongBlinkLED1(void)
{
    // LEDを1秒間隔で点滅させる
    //   点灯950ms、消灯50ms
    sLED_1.Blink(950, 50);
}

void AppLEDSetHalfBlinkLED1(void)
{
    // LEDを200ms間隔で点滅させる
    sLED_1.Blink(100, 100);
}

void AppLEDSetShortBlinkLED1(void)
{
    // LEDを1秒間隔で点滅させる
    //   点灯50ms、消灯950ms
    sLED_1.Blink(50, 950);
}

//
// LED点灯／消灯のための状態管理
//
static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

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
    if (AppEventHandlerButtonPushedLong() == false) {
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

void AppLEDSetToggleLED2(bool b)
{
    // Set lock status LED back to show state of lock.
    sLED_2.Set(b);
    sLED_3.Set(false);
    sLED_4.Set(false);
}

void AppLEDSetBlinkAllLED(void)
{
    sLED_1.Set(false);
    sLED_2.Set(false);
    sLED_3.Set(false);
    sLED_4.Set(false);

    sLED_1.Blink(500);
    sLED_2.Blink(500);
    sLED_3.Blink(500);
    sLED_4.Blink(500);
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
