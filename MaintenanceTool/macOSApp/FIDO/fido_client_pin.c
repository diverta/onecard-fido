//
//  fido_client_pin.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/04/06.
//
#include <stdlib.h>
#include "fido_crypto.h"
#include "fido_client_pin.h"
#include "FIDODefines.h"

bool fido_client_pin_generate_pinauth(char *new_pin, char *old_pin, bool change_pin)
{
    // pinHashEncを生成
    if (change_pin) {
        if (generate_pin_hash_enc(old_pin) != CTAP1_ERR_SUCCESS) {
            return false;
        }
    }
    // newPinEncを生成
    if (generate_new_pin_enc(new_pin) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // pinAuthを生成
    if (generate_pin_auth(change_pin) != CTAP1_ERR_SUCCESS) {
        return false;
    }
    // 処理成功
    return true;
}
