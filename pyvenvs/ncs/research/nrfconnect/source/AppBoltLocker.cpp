/* 
 * File:   AppBoltLocker.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 15:22
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppBoltLocker);

#include "AppProcess.h"
#include "AppEventHandler.h"
#include "AppLED.h"

//
// 状態保持用変数
//
enum Action_t {
    LOCK_ACTION = 0,
    UNLOCK_ACTION,
    INVALID_ACTION
};

enum Actor_t {
    REAL_ACTOR = 0,
    SIMULATION_ACTOR,
    TIMER_ACTOR,
    INVALID_ACTOR
};

enum State_t {
    kState_LockingInitiated = 0,
    kState_LockingCompleted,
    kState_UnlockingInitiated,
    kState_UnlockingCompleted,
};

static State_t  mState;
static bool     mAutoLockTimerArmed;
static bool     mAutoRelock;
static uint32_t mAutoLockDuration;

static Actor_t  mCurrentActor;
static Action_t mCurrentAction;

static void InitializeValues(void)
{
    mState              = kState_LockingCompleted;
    mAutoLockTimerArmed = false;
    mAutoRelock         = false;
    mAutoLockDuration   = 0;
}

static bool IsUnlocked()
{
    return (mState == kState_UnlockingCompleted);
}

//
// ボタン押下により擬似的にメッセージを発生させるため、
// 手動でCHIPステータスを更新する必要があります
//
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/attribute-storage.h>

static void UpdateClusterState()
{
    uint8_t newValue = !IsUnlocked();

    // write the new on/off value
    chip::EndpointId endpoint = 1;
    EmberAfStatus status = emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS) {
        LOG_ERR("Updating on/off %x", status);
    }
}

//
// タイマー関連
//   施錠／解錠用のモーター類動作を疑似するため
//   ２秒のタイマーを設置します
// 
#define ACTUATOR_MOVEMENT_PERIOS_MS 2000

static k_timer sLockTimer;

typedef void (*TimerEventHandlerFunc)(void);
static void AutoReLockTimerEventHandler(void);
static void ActuatorMovementTimerEventHandler(void);

static void TimerEventHandler(k_timer *timer)
{
    // タイマー満了時は、AppEventHandler経由で、
    // ２秒経過後の関数を実行する
    (void)timer;
    TimerEventHandlerFunc func = mAutoLockTimerArmed ? AutoReLockTimerEventHandler : ActuatorMovementTimerEventHandler;
    AppEventHandlerLockActionEventPost((void *)func);
}

static void InitializeTimer(void)
{
    k_timer_init(&sLockTimer, TimerEventHandler, nullptr);
}

static void StartTimer(uint32_t aTimeoutMs)
{
    k_timer_start(&sLockTimer, K_MSEC(aTimeoutMs), K_NO_WAIT);
}

static void CancelTimer(void)
{
    k_timer_stop(&sLockTimer);
}

//
// 解錠・施錠動作を発生させる処理
//
static void ActionInitiated(Action_t aAction, Actor_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    if (aAction == LOCK_ACTION) {
        LOG_INF("Lock Action has been initiated %s",
                aActor == SIMULATION_ACTOR ? "by BUTTON" : "");

    } else if (aAction == UNLOCK_ACTION) {
        LOG_INF("Unlock Action has been initiated %s",
                aActor == SIMULATION_ACTOR ? "by BUTTON" : "");
    }

    // Start flashing the LEDs rapidly to indicate action initiation.
    AppLEDSetBlinkLED2();
}

static bool InitiateAction(Actor_t aActor, Action_t aAction)
{
    bool action_initiated = false;
    State_t new_state;

    // Initiate Lock/Unlock Action only when the previous one is complete.
    if (mState == kState_LockingCompleted && aAction == UNLOCK_ACTION) {
        action_initiated = true;
        mCurrentActor    = aActor;
        mCurrentAction   = aAction;
        new_state        = kState_UnlockingInitiated;

    } else if (mState == kState_UnlockingCompleted && aAction == LOCK_ACTION) {
        action_initiated = true;
        mCurrentActor    = aActor;
        mCurrentAction   = aAction;
        new_state        = kState_LockingInitiated;

    } else {
        mCurrentAction   = INVALID_ACTION;
    }

    if (action_initiated) {
        if (mAutoLockTimerArmed && new_state == kState_LockingInitiated) {
            // If auto lock timer has been armed and someone initiates locking,
            // cancel the timer and continue as normal.
            mAutoLockTimerArmed = false;
            CancelTimer();
        }

        StartTimer(ACTUATOR_MOVEMENT_PERIOS_MS);

        // Since the timer started successfully, update the state and trigger callback
        mState = new_state;
        ActionInitiated(aAction, aActor);
    }

    return action_initiated;
}

//
// イベントキューからのコールバック
//
void AppBoltLockerSendLockAction(bool simulated, void *func)
{
    // タイマー満了時の場合
    TimerEventHandlerFunc f = (TimerEventHandlerFunc)func;
    if (f == AutoReLockTimerEventHandler) {
        AutoReLockTimerEventHandler();
        return;
    }
    if (f == ActuatorMovementTimerEventHandler) {
        ActuatorMovementTimerEventHandler();
        return;
    }

    Action_t action = INVALID_ACTION;
    Actor_t  actor  = INVALID_ACTOR;

    if (simulated == false) {
        action = mCurrentAction;
        actor  = mCurrentActor;
    } else {
        action = IsUnlocked() ? LOCK_ACTION : UNLOCK_ACTION;
        actor  = SIMULATION_ACTOR;
    }
    if (action == INVALID_ACTION) {
        return;
    }

    if (InitiateAction(actor, action) == false) {
        LOG_INF("Action is already in progress or active.");
    }
}

static void AutoReLockTimerEventHandler(void)
{
    // Make sure auto lock timer is still armed.
    if (mAutoLockTimerArmed == false) {
        return;
    }
    mAutoLockTimerArmed = false;

    LOG_INF("Auto Re-Lock has been triggered!");
    InitiateAction(TIMER_ACTOR, LOCK_ACTION);
}

static void ActionCompleted(Action_t aAction, Actor_t aActor)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == LOCK_ACTION) {
        LOG_INF("Lock Action has been completed");
        AppLEDSetToggleLED2(true);

    } else if (aAction == UNLOCK_ACTION) {
        LOG_INF("Unlock Action has been completed");
        AppLEDSetToggleLED2(false);
    }

    if (aActor == SIMULATION_ACTOR) {
        LOG_INF("CHIP cluster state will update manually");
        UpdateClusterState();
    }
}

static void ActuatorMovementTimerEventHandler(void)
{
    Action_t actionCompleted = INVALID_ACTION;

    if (mState == kState_LockingInitiated) {
        mState = kState_LockingCompleted;
        actionCompleted = LOCK_ACTION;

    } else if (mState == kState_UnlockingInitiated) {
        mState = kState_UnlockingCompleted;
        actionCompleted = UNLOCK_ACTION;
    }

    if (actionCompleted != INVALID_ACTION) {
        ActionCompleted(actionCompleted, mCurrentActor);

        if (mAutoRelock && actionCompleted == UNLOCK_ACTION) {
            // Start the timer for auto relock
            StartTimer(mAutoLockDuration * 1000);
            mAutoLockTimerArmed = true;
            LOG_INF("Auto Re-lock enabled. Will be triggered in %u seconds", mAutoLockDuration);
        }
    }
}

//
// CHIPプラットフォームからのコールバック
//
#include <support/logging/CHIPLogging.h>

using namespace ::chip;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint16_t size, uint8_t * value)
{
    (void)mask;
    (void)manufacturerCode;
    (void)type;
    (void)size;

    if (clusterId != ZCL_ON_OFF_CLUSTER_ID) {
        ChipLogProgress(Zcl, "Unknown cluster ID: %d", clusterId);
        return;
    }
    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID) {
        ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
        return;
    }

    LOG_INF("InitiateAction called: endpoint=%d", endpoint);
    InitiateAction(REAL_ACTOR, *value ? LOCK_ACTION : UNLOCK_ACTION);
}

void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    LOG_INF("CHIP cluster state will update: endpoint=%d", endpoint);
    UpdateClusterState();
}

//
// 外部公開関数
//
void AppBoltLockerInitialize(void)
{
    InitializeTimer();
    InitializeValues();
}

bool AppBoltLockerIsLocked(void)
{
    return !IsUnlocked();
}
