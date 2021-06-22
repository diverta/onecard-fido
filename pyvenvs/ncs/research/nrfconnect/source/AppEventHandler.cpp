/* 
 * File:   AppEventHandler.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 11:59
 */
#include <zephyr.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/server/Server.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppEventHandler);

#include "AppBoltLocker.h"
#include "AppProcess.h"
#include "AppDFU.h"

// TODO: 不要な機能（後日削除します）
#include "ThreadUtil.h" 

//
// for CHIP classes
//   ConnectivityMgr(), ConfigurationMgr()
//
using namespace ::chip::DeviceLayer;

//
// イベント関連
//
struct AppEvent;

typedef void (*EventHandler)(AppEvent *);
struct AppEvent {
    // 全イベントに共通の属性
    EventHandler Handler;
    void        *HandlerParam;

    // ボタンイベント専用の属性
    uint8_t      ButtonAction;
};

enum ButtonPushStatus_t {
    kButtonPushedNone  = 0,
    kButtonPushedShort = 0,
    kButtonPushedLong,
};

ButtonPushStatus_t mButtonPushStatus = kButtonPushedNone;
bool mFunctionTimerActive   = false;

bool AppEventHandlerButtonPushedLong(void)
{
    return (mButtonPushStatus == kButtonPushedLong);
}

//
// イベントキュー関連
//
#define APP_EVENT_QUEUE_SIZE 10
K_MSGQ_DEFINE(sApplicationEventQueue, sizeof(AppEvent), APP_EVENT_QUEUE_SIZE, alignof(AppEvent));

static void PostEvent(AppEvent *aEvent)
{
    if (k_msgq_put(&sApplicationEventQueue, aEvent, K_NO_WAIT)) {
        LOG_WRN("Failed to post event to app task event queue");
    }
}

static void DispatchEvent(AppEvent *aEvent)
{
    if (aEvent->Handler) {
        aEvent->Handler(aEvent);

    } else {
        LOG_INF("Event received with no handler. Dropping event.");
    }
}

int AppEventHandlerDispatch(void)
{
    AppEvent event = {};

    int ret = k_msgq_get(&sApplicationEventQueue, &event, K_MSEC(10));
    while (ret == 0) {
        DispatchEvent(&event);
        ret = k_msgq_get(&sApplicationEventQueue, &event, K_NO_WAIT);
    }

    return 0;
}

//
// ボタン押下イベント処理
//
#define BUTTON_1      DK_BTN1
#define BUTTON_2      DK_BTN2
#define BUTTON_3      DK_BTN3
#define BUTTON_4      DK_BTN4

#define BUTTON_1_MASK DK_BTN1_MSK
#define BUTTON_2_MASK DK_BTN2_MSK
#define BUTTON_3_MASK DK_BTN3_MSK
#define BUTTON_4_MASK DK_BTN4_MSK

#define BUTTON_PUSH_EVENT       1
#define BUTTON_RELEASE_EVENT    0

static void FunctionHandler(AppEvent * aEvent);
static void LockActionEventHandler(AppEvent * aEvent);
static void StartThreadHandler(AppEvent * aEvent);
static void StartBLEAdvertisementHandler(AppEvent * aEvent);

static void ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent event;

    if (BUTTON_1_MASK & has_changed) {
        event.ButtonAction = (BUTTON_1_MASK & button_state) ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;
        event.Handler      = FunctionHandler;
        PostEvent(&event);
    }

    if (BUTTON_2_MASK & button_state & has_changed) {
        event.ButtonAction = BUTTON_PUSH_EVENT;
        event.Handler      = LockActionEventHandler;
        PostEvent(&event);
    }

    if (BUTTON_3_MASK & button_state & has_changed) {
        event.ButtonAction = BUTTON_PUSH_EVENT;
        event.Handler      = StartThreadHandler;
        PostEvent(&event);
    }

    if (BUTTON_4_MASK & button_state & has_changed) {
        event.ButtonAction = BUTTON_PUSH_EVENT;
        event.Handler      = StartBLEAdvertisementHandler;
        PostEvent(&event);
    }
}

static bool InitializeButtons(void)
{
    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret) {
        LOG_ERR("dk_buttons_init() returns %d", ret);
        return false;
    }
    return true;
}

//
// ボタン長押し判定用タイマー
//
#define FACTORY_RESET_TRIGGER_TIMEOUT       3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000

static k_timer sFunctionTimer;
static void FunctionTimerEventHandler(AppEvent *aEvent);

static void TimerEventHandler(k_timer *timer)
{
    (void)timer;
    AppEvent event = {
        .Handler = FunctionTimerEventHandler
    };
    PostEvent(&event);
}

static void InitializeTimer(void)
{
    k_timer_init(&sFunctionTimer, TimerEventHandler, nullptr);
}

static void StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
}

static void CancelTimer(void)
{
    k_timer_stop(&sFunctionTimer);
}

//
// イベント処理群
//
static void ButtonPushedShortHandler(AppEvent *aEvent)
{
    // trigger a software update.
    AppDFUEnableFirmwareUpdate();
}

static void ButtonPushedLongHandler(AppEvent *aEvent)
{
    // Actually trigger Factory Reset
    ConfigurationMgr().InitiateFactoryReset();
}

static void FunctionHandler(AppEvent *aEvent)
{
    // To trigger software update: 
    //   press the BUTTON_1 button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
    // To initiate factory reset: 
    //   press the BUTTON_1 for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    //   All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: 
    //   release the BUTTON_1 once all LEDs start blinking within the
    //   FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonAction == BUTTON_PUSH_EVENT) {
        if (!mFunctionTimerActive && mButtonPushStatus == kButtonPushedNone) {
            StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
            mFunctionTimerActive = true;
            mButtonPushStatus = kButtonPushedShort;
        }

    } else {
        // If the button was released before factory reset got initiated
        if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedShort) {
            CancelTimer();
            mFunctionTimerActive = false;
            mButtonPushStatus = kButtonPushedNone;
            ButtonPushedShortHandler(aEvent);

        } else if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedLong) {
            // Set lock status LED back to show state of lock.
            AppLEDSetToggleLED2(AppBoltLockerIsLocked());
            CancelTimer();
            mFunctionTimerActive = false;

            // Change the function to none selected since factory reset has been canceled.
            mButtonPushStatus = kButtonPushedNone;
            LOG_INF("Factory Reset has been Canceled");
        }
    }
}

static void LockActionEventHandler(AppEvent *aEvent)
{
    //
    // TODO: MatterコマンドがCHIPネットワークから送信されたのと
    //       等価の処理を行う（仮実装）
    //
    (void)aEvent;
}

static void StartThreadHandler(AppEvent *aEvent)
{
    //
    // TODO: 不要な機能（後日削除します）
    //
    (void)aEvent;
    if (AddTestPairing() != CHIP_NO_ERROR) {
        LOG_ERR("Failed to add test pairing");
    }

    if (!ConnectivityMgr().IsThreadProvisioned()) {
        StartDefaultThreadNetwork();
        LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration.");
    } else {
        LOG_INF("Device is commissioned to a Thread network.");
    }
}

static void StartBLEAdvertisementHandler(AppEvent *aEvent)
{
    // In case of having software update enabled, allow on starting BLE advertising after Thread provisioning.
    (void)aEvent;
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

static void FunctionTimerEventHandler(AppEvent *aEvent)
{
    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    (void)aEvent;
    if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedShort) {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        mFunctionTimerActive = true;
        mButtonPushStatus = kButtonPushedLong;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        AppLEDSetBlinkAllLED();

    } else if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedLong) {
        mButtonPushStatus = kButtonPushedNone;
        ButtonPushedLongHandler(aEvent);
    }
}

//
// 初期化処理
//
bool AppEventHandlerInit(void)
{
    if (InitializeButtons() == false) {
        return false;
    }

    InitializeTimer();
    return true;
}
