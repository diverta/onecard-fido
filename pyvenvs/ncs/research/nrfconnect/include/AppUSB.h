/* 
 * File:   AppUSB.h
 * Author: makmorit
 *
 * Created on 2021/06/29, 12:41
 */
#pragma once

//
// 関数群
//
bool        AppUSBInitialize(void);
bool        AppUSBHidSendReport(uint8_t *data, size_t size);
