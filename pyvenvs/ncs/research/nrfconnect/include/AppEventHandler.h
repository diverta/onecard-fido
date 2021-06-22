/* 
 * File:   AppEventHandler.h
 * Author: makmorit
 *
 * Created on 2021/06/17, 11:59
 */
#pragma once

//
// 関数群
//
bool        AppEventHandlerInit(void);
int         AppEventHandlerDispatch(void);
bool        AppEventHandlerButtonPushedLong(void);
void        AppEventHandlerLockActionEventPost(void *param);
