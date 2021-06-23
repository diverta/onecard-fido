/* 
 * File:   AppLED.h
 * Author: makmorit
 *
 * Created on 2021/06/23, 12:47
 */
#pragma once

//
// 関数群
//
void        AppLEDInit(void);
void        AppLEDAnimate(void);
void        AppLEDKeepOnLED1(void);
void        AppLEDSetLongBlinkLED1(void);
void        AppLEDSetHalfBlinkLED1(void);
void        AppLEDSetShortBlinkLED1(void);
void        AppLEDSetBlinkLED2(void);
void        AppLEDSetToggleLED2(bool b);
void        AppLEDSetBlinkAllLED(void);
