/* 
 * File:   AppProcess.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 10:38
 */
#include "AppTask.h"

int applicationProcess(void)
{
    return GetAppTask().StartApp();
}
