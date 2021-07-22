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
#include "AppHIDCommand.h"
#include "AppLED.h"
#include "AppUSB.h"

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
static bool sIsBLEAdvertizeEnabled   = false;

static void SetLED2Status(void)
{
    // 青色LEDと緑色LEDが同時点灯しないようにする
    if (sIsBLEAdvertizeEnabled) {
        AppLEDSetToggleLED2(false);
    } else {
        AppLEDSetToggleLED2(AppBoltLockerIsLocked());
    }
}

static void FactoryResetTriggered(void)
{
    // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
    AppLEDSetBlinkAllLED();
    sIsFactoryResetTriggered = true;
    LOG_INF("Factory Reset Triggered. Release button within 3 seconds to cancel.");
}

static void FactoryResetCancelled(void)
{
    // Set lock status LED back to show state of lock.
    SetLED2Status();
    AppLEDSetToggleLED3(AppBoltLockerAutoRelockEnabled());
    AppLEDSetToggleLED4(sIsBLEAdvertizeEnabled);
    sIsFactoryResetTriggered = false;
    LOG_INF("Factory Reset has been Canceled");
}

static void CheckBLEAdvertizeEnabled(void)
{
    // 現在の状態を保持
    static bool enabled = false;

    if (!enabled && sIsBLEAdvertizeEnabled) {
        // アドバタイズOff-->Onに遷移時は
        // 青色LEDを点灯し、緑色LEDを消す
        SetLED2Status();
        AppLEDSetToggleLED4(true);

    } else if (enabled && !sIsBLEAdvertizeEnabled) {
        // アドバタイズOn-->Offに遷移時は
        // 青色LEDを消灯し、緑色LEDを復活
        AppLEDSetToggleLED4(false);
        SetLED2Status();
    }

    // 現在の状態を更新
    enabled = sIsBLEAdvertizeEnabled;
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
        // 青色LEDの制御
        CheckBLEAdvertizeEnabled();

        // 橙色LEDの点灯制御
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
        sIsBLEAdvertizeEnabled   = ConnectivityMgr().IsBLEAdvertisingEnabled();
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

    uint64_t nowUS = chip::System::Clock::GetMonotonicMicroseconds();
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

    // USB初期化処理
    if (AppUSBInitialize() == false) {
        return false;
    }

    // Confirm firmware image
    AppDFUConfirmFirmwareImage();

    // Bolt lock manager
    AppBoltLockerInitialize();

    // Lock状態をLEDに反映
    AppLEDSetToggleLED2(AppBoltLockerIsLocked());
    AppLEDSetToggleLED3(AppBoltLockerAutoRelockEnabled());

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

//
// 業務処理群
//
static void StartBLEAdvertiseForCommission(void)
{
    // In case of having software update enabled, allow on starting BLE advertising after Thread provisioning.
    if (ConnectivityMgr().IsThreadProvisioned() && AppDFUFirmwareUpdateEnabled() == false) {
        LOG_INF("BLE advertisement not started - device is commissioned to a Thread network.");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled()) {
        LOG_INF("BLE Advertisement is already enabled");
        return;
    }

    if (OpenDefaultPairingWindow(chip::ResetAdmins::kNo) == CHIP_NO_ERROR) {
        LOG_INF("Enabled BLE Advertisement");
    } else {
        LOG_ERR("OpenDefaultPairingWindow() failed");
    }
}

static void ToggleAutoRelockEnabled(void)
{
    // 解錠状態では、自動再施錠の設定変更が
    // 出来ないようガードする
    if (AppBoltLockerIsLocked() == false) {
        LOG_WRN("Auto Re-lock cannot be toggled while unlocked");
        return;
    }
    
    if (AppBoltLockerAutoRelockEnabled()) {
        // 自動再施錠をしないよう設定
        AppBoltLockerEnableAutoRelock(false);
        LOG_INF("Auto Re-lock is turned to disabled");
        
    } else {
        // ３０秒後に自動再施錠するよう設定
        AppBoltLockerEnableAutoRelock(true);
        AppBoltLockerSetAutoLockDuration(30);
        LOG_INF("Auto Re-lock is turned to enabled");
    }
    AppLEDSetToggleLED3(AppBoltLockerAutoRelockEnabled());
}

void AppProcessButton1PushedShort(void)
{
    // コミッショニングのためのBLEアドバタイジング開始処理
    StartBLEAdvertiseForCommission();
}

void AppProcessButton1Pushed3Seconds(void)
{
    FactoryResetTriggered();
}

void AppProcessButton1PushedSemiLong(void)
{
    FactoryResetCancelled();
}

void AppProcessButton1PushedLong(void)
{
    // Actually trigger Factory Reset
    ConfigurationMgr().InitiateFactoryReset();
}

void AppProcessButton2PushedShort(void)
{
    // CHIPから解錠・施錠コマンドを受信したのと等価の処理を行う
    AppBoltLockerSimulateLockAction();
}

void AppProcessButton3PushedShort(void)
{
    // 解錠後の自動施錠実行設定（On/Off）を変更
    ToggleAutoRelockEnabled();
}

void AppProcessButton4PushedShort(void)
{
    // trigger a software update.
    AppDFUEnableFirmwareUpdate();
}

void AppProcessActionInitiated(void)
{
    // 解錠／施錠動作が開始された時の処理
    AppLEDSetBlinkLED2();
}

void AppProcessActionCompleted(bool isLockAction)
{
    // 解錠／施錠動作が完了した時の処理
    AppLEDSetToggleLED2(isLockAction);
}

void AppProcessUSBHIDCommand(uint8_t command)
{
    // USB HID I/Fからコマンド投入時の処理
    switch (command) {
        case 0xcf:
            // リセット処理を行う（工場出荷状態に戻す）
            ConfigurationMgr().InitiateFactoryReset();
            break;
        case 0xce:
            // CHIPから解錠・施錠コマンドを受信したのと等価の処理を行う
            AppBoltLockerSimulateLockAction();
            break;
        case 0xcd:
            // 解錠後の自動施錠実行設定（On/Off）を変更
            ToggleAutoRelockEnabled();
            break;
        default:
            LOG_ERR("HID Command 0x%02x is invalid", command);
            break;
    }
}

void AppProcessUSBConfigured(void)
{
    LOG_INF("USB connected");
}

void AppProcessUSBDisconnected(void)
{
    LOG_INF("USB disconnected");
}

void AppProcessHIDReportReceived(uint8_t *data, size_t size)
{
    AppHIDCommandReceived(data, size);
}

void AppProcessHIDReportSent(void)
{
}
