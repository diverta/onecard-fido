/* 
 * File:   AppEventHandler.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 11:59
 */
#include <zephyr.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppEventHandler);

#include "AppProcess.h"

//
// イベント関連
//
struct AppEvent;

typedef void (*EventHandler)(AppEvent *);
struct AppEvent {
    // 全イベントに共通の属性
    EventHandler Handler;

    // ボタンイベント専用の属性
    uint8_t      ButtonAction;

    // イベントコールバック／引数
    void       (*Callback)(void *);
    void        *CallbackParam;
};

enum ButtonPushStatus_t {
    kButtonPushedNone  = 0,
    kButtonPushedShort,
    kButtonPushedLong,
};

static ButtonPushStatus_t mButtonPushStatus = kButtonPushedNone;
static bool mFunctionTimerActive = false;

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

#define BUTTON_NONE_EVENT       0
#define BUTTON_PUSH_EVENT       1
#define BUTTON_RELEASE_EVENT    2

static void Button1EventHandler(AppEvent * aEvent);
static void Button2EventHandler(AppEvent * aEvent);
static void Button3EventHandler(AppEvent * aEvent);
static void Button4EventHandler(AppEvent * aEvent);

static void ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent event = {
        .Handler       = nullptr,
        .ButtonAction  = BUTTON_NONE_EVENT,
        .Callback      = nullptr,
        .CallbackParam = nullptr
    };

    if (BUTTON_1_MASK & has_changed) {
        event.Handler      = Button1EventHandler;
        event.ButtonAction = (BUTTON_1_MASK & button_state) ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;
        PostEvent(&event);
    }

    if (BUTTON_2_MASK & button_state & has_changed) {
        event.Handler      = Button2EventHandler;
        event.ButtonAction = BUTTON_PUSH_EVENT;
        PostEvent(&event);
    }

    if (BUTTON_3_MASK & button_state & has_changed) {
        event.Handler      = Button3EventHandler;
        event.ButtonAction = BUTTON_PUSH_EVENT;
        PostEvent(&event);
    }

    if (BUTTON_4_MASK & button_state & has_changed) {
        event.Handler      = Button4EventHandler;
        event.ButtonAction = BUTTON_PUSH_EVENT;
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
        .Handler       = FunctionTimerEventHandler,
        .ButtonAction  = BUTTON_NONE_EVENT,
        .Callback      = nullptr,
        .CallbackParam = nullptr
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
// カスタムイベント処理
//   他の業務処理スレッドと、このモジュールで管理する
//   イベントとの同期を取るのが目的
//
static void FunctionEventPostHandler(AppEvent *aEvent)
{
    if (aEvent->Callback == nullptr) {
        return;
    }
    (*aEvent->Callback)(aEvent->CallbackParam);
}

void AppEventHandlerFunctionEventPost(void (*func)(void *), void *param)
{
    AppEvent event = {
        .Handler       = FunctionEventPostHandler,
        .ButtonAction  = BUTTON_NONE_EVENT,
        .Callback      = func,
        .CallbackParam = param
    };
    PostEvent(&event);
}

//
// イベント処理群
//
static void Button1EventHandler(AppEvent *aEvent)
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
            AppProcessButton1PushedShort();

        } else if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedLong) {
            CancelTimer();
            mFunctionTimerActive = false;
            mButtonPushStatus = kButtonPushedNone;
            AppProcessButton1PushedSemiLong();
        }
    }
}

static void Button2EventHandler(AppEvent *aEvent)
{
    (void)aEvent;
    AppProcessButton2PushedShort();
}

static void Button3EventHandler(AppEvent *aEvent)
{
    (void)aEvent;
    AppProcessButton3PushedShort();
}

static void Button4EventHandler(AppEvent *aEvent)
{
    (void)aEvent;
    AppProcessButton4PushedShort();
}

static void FunctionTimerEventHandler(AppEvent *aEvent)
{
    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    (void)aEvent;
    if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedShort) {
        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        mFunctionTimerActive = true;
        mButtonPushStatus = kButtonPushedLong;
        AppProcessButton1Pushed3Seconds();

    } else if (mFunctionTimerActive && mButtonPushStatus == kButtonPushedLong) {
        mButtonPushStatus = kButtonPushedNone;
        AppProcessButton1PushedLong();
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
