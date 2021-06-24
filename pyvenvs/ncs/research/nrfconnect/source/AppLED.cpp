/* 
 * File:   AppLED.cpp
 * Author: makmorit
 *
 * Created on 2021/06/23, 12:47
 */
#include <zephyr.h>
#include <dk_buttons_and_leds.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppLED);

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

void AppLEDSetBlinkLED2(void)
{
    // Start flashing the LEDs rapidly to indicate action initiation.
    sLED_2.Blink(50, 50);
}

void AppLEDSetToggleLED2(bool b)
{
    // Set lock status LED back to show state of lock.
    sLED_2.Set(b);
}

void AppLEDSetToggleLED3(bool b)
{
    sLED_3.Set(b);
}

void AppLEDSetToggleLED4(bool b)
{
    sLED_4.Set(b);
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
