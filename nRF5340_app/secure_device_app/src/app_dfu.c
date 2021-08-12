/* 
 * File:   app_dfu.c
 * Author: makmorit
 *
 * Created on 2021/08/11, 15:42
 */
#include <zephyr/types.h>
#include <zephyr.h>
#include <dfu/mcuboot.h>

#define LOG_LEVEL LOG_LEVEL_DBG
#include <logging/log.h>
#include <logging/log_ctrl.h>
LOG_MODULE_REGISTER(app_dfu);

#include "app_board.h"
#include "app_timer.h"
#include "app_usb.h"

bool app_dfu_prepare_for_bootloader(void)
{
    // USBを停止
    if (app_usb_deinitialize() == false) {
        return false;
    }

    // ブートローダーモードに遷移させるため、
    // GPREGRETレジスターにその旨の値を設定
    app_board_prepare_for_bootloader_mode();

    // ブートローダーに制御を移す
    //   2秒待機後、システムリセットを実行
    LOG_INF("Entering to bootloader mode");
    LOG_PROCESS();
    app_timer_start_for_idling(2000, APEVT_ENTER_TO_BOOTLOADER);
    return true;
}

bool app_dfu_commit(void)
{
    // DFUによる変更が行われたかどうかチェック
    //   BOOT_SWAP_TYPE_REVERTの場合、
    //   変更が行われたと判断
    int swap_type = mcuboot_swap_type();
    if (swap_type != BOOT_SWAP_TYPE_REVERT) {
        LOG_DBG("Ignore swap type (%d)", swap_type);
        return false;
    }

    // DFUによる変更内容のコミットを指示
    int ret = boot_write_img_confirmed();
    if (ret != 0) {
        LOG_ERR("boot_write_img_confirmed returns %d", ret);
        return false;
    }

    // ブートローダーに制御を移す
    //   2秒待機後、システムリセットを実行
    LOG_INF("Boot write image confirmed");
    LOG_PROCESS();
    app_timer_start_for_idling(2000, APEVT_ENTER_TO_BOOTLOADER);
    return true;
}
