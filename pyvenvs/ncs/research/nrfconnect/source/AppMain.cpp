/* 
 * File:   AppMain.cpp
 * Author: makmorit
 *
 * Created on 2021/06/16, 15:51
 */
#include "AppProcess.h"

#include <logging/log.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

LOG_MODULE_REGISTER(AppMain);

//
// Initialize CHIP layer
//
using namespace ::chip;
using namespace ::chip::DeviceLayer;

static int initializeCHIP(void)
{
    int ret = chip::Platform::MemoryInit();
    if (ret != CHIP_NO_ERROR) {
        LOG_ERR("Platform::MemoryInit() returns %d", ret);
        return ret;
    }

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR) {
        LOG_ERR("PlatformMgr().InitChipStack() returns %d", ret);
        return ret;
    }
    LOG_INF("Init CHIP stack done");

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR) {
        LOG_ERR("PlatformMgr().StartEventLoopTask() returns %d", ret);
        return ret;
    }
    LOG_INF("CHIP task started");

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR) {
        LOG_ERR("ThreadStackMgr().InitThreadStack() returns %d", ret);
        return ret;
    }

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
    if (ret != CHIP_NO_ERROR) {
        LOG_ERR("ConnectivityMgr().SetThreadDeviceType() returns %d", ret);
        return ret;
    }
    LOG_INF("Init Thread stack done");
    return CHIP_NO_ERROR;
}

int applicationMain(void)
{
    k_thread_priority_set(k_current_get(), K_PRIO_COOP(CONFIG_NUM_COOP_PRIORITIES - 1));

    int ret = initializeCHIP();
    if (ret != CHIP_NO_ERROR) {
        return ret;
    }

    return AppProcessMain();
}
