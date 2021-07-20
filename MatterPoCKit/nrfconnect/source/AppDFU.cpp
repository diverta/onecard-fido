/* 
 * File:   AppDFU.cpp
 * Author: makmorit
 *
 * Created on 2021/06/17, 15:29
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppDFU);

// MCUMgr BT FOTA includes
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_SMP_BT
#include <mgmt/mcumgr/smp_bt.h>
#endif
#ifdef CONFIG_BOOTLOADER_MCUBOOT
#include <dfu/mcuboot.h>
#endif

bool mSoftwareUpdateEnabled = false;

void AppDFUConfirmFirmwareImage(void)
{
#ifdef CONFIG_BOOTLOADER_MCUBOOT
    // Check if the image is run in the REVERT mode and eventually
    // confirm it to prevent reverting on the next boot.
    if (mcuboot_swap_type() == BOOT_SWAP_TYPE_REVERT) {
        if (boot_write_img_confirmed()) {
            LOG_ERR("Confirming firmware image failed, it will be reverted on the next boot.");

        } else {
            LOG_INF("New firmware image confirmed.");
        }
    }
#endif
}

bool AppDFUFirmwareUpdateEnabled(void)
{
    return mSoftwareUpdateEnabled;
}

#if defined(CONFIG_MCUMGR_SMP_BT) && defined(CONFIG_MCUMGR_CMD_IMG_MGMT) && defined(CONFIG_MCUMGR_CMD_OS_MGMT)
static int SoftwareUpdateConfirmationHandler(uint32_t offset, uint32_t size, void *arg)
{
    // For now just print update progress and confirm data chunk without any additional checks.
    LOG_INF("Software update progress %d B / %d B", offset, size);
    return 0;
}
#endif

void AppDFUEnableFirmwareUpdate(void)
{
#if defined(CONFIG_MCUMGR_SMP_BT) && defined(CONFIG_MCUMGR_CMD_IMG_MGMT) && defined(CONFIG_MCUMGR_CMD_OS_MGMT)
    if (mSoftwareUpdateEnabled == false) {
        mSoftwareUpdateEnabled = true;
        os_mgmt_register_group();
        img_mgmt_register_group();
        img_mgmt_set_upload_cb(SoftwareUpdateConfirmationHandler, NULL);
        smp_bt_register();
        LOG_INF("Enabled software update");

    } else {
        LOG_INF("Software update is already enabled");
    }

#else
    LOG_INF("Software update is disabled");
#endif
}
