/* 
 * File:   ccid_process.c
 * Author: makmorit
 *
 * Created on 2022/04/29, 9:07
 */
#include "ccid_apdu.h"
#include "ccid_piv.h"
#include "ccid_openpgp.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

#ifdef FIDO_ZEPHYR
fido_log_module_register(ccid_process);
#endif
