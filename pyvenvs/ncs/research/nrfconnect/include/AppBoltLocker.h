/* 
 * File:   AppBoltLocker.h
 * Author: makmorit
 *
 * Created on 2021/06/17, 15:22
 */
#pragma once

//
// 関数群
//
void        AppBoltLockerInitialize(void);
bool        AppBoltLockerIsLocked(void);
void        AppBoltLockerSendLockAction(bool simulated, void *param);
