/* 
 * File:   atecc_sign.c
 * Author: makmorit
 *
 * Created on 2020/08/24, 9:14
 */
#include <stdlib.h>
#include <string.h>

#include "atecc_command.h"
#include "atecc_nonce.h"
#include "atecc_util.h"

// 業務処理／HW依存処理間のインターフェース
#include "fido_platform.h"

//
// 署名関連
//
static bool atecc_sign_base(uint8_t mode, uint16_t key_id, uint8_t *signature)
{
    if (signature == NULL) {
        fido_log_error("atecc_sign_base failed: BAD_PARAM");
        return false;
    }

    // Build sign command
    ATECC_PACKET packet;
    packet.param1 = mode;
    packet.param2 = key_id;
    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_sign(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    if (packet.data[ATECC_IDX_COUNT] > 4) {
        memcpy(signature, &packet.data[ATECC_IDX_RSP_DATA], packet.data[ATECC_IDX_COUNT] - ATECC_PACKET_OVERHEAD);
    }

    return true;
}

bool atecc_sign(uint16_t key_id, const uint8_t *msg, uint8_t *signature)
{
    uint8_t nonce_target = NONCE_MODE_TARGET_TEMPKEY;
    uint8_t sign_source = SIGN_MODE_SOURCE_TEMPKEY;

    // Make sure RNG has updated its seed
    if (atecc_random(NULL) == false) {
        return false;
    }

    // Load message into device
    if (atecc_device_ref()->mCommands->dt == ATECC608A) {
        // Use the Message Digest Buffer for the ATECC608A
        nonce_target = NONCE_MODE_TARGET_MSGDIGBUF;
        sign_source = SIGN_MODE_SOURCE_MSGDIGBUF;
    }
    if (atecc_nonce_load(nonce_target, msg, 32) == false) {
        return false;
    }

    // Sign the message
    if (atecc_sign_base(SIGN_MODE_EXTERNAL | sign_source, key_id, signature) == false) {
        return false;
    }

    return true;
}

//
// 署名検証関連
//
static bool atecc_verify(uint8_t mode, uint16_t key_id, const uint8_t *signature, const uint8_t *public_key, const uint8_t *other_data, uint8_t *mac)
{
    if (signature == NULL) {
        fido_log_error("atecc_verify failed: BAD_PARAM");
        return false;
    }

    uint8_t verify_mode = (mode & VERIFY_MODE_MASK);
    if (verify_mode == VERIFY_MODE_EXTERNAL && public_key == NULL) {
        fido_log_error("atecc_verify failed: BAD_PARAM");
        return false;
    }
    if ((verify_mode == VERIFY_MODE_VALIDATE || verify_mode == VERIFY_MODE_INVALIDATE) && other_data == NULL) {
        fido_log_error("atecc_verify failed: BAD_PARAM");
        return false;
    }

    // Build the verify command
    ATECC_PACKET packet;
    packet.param1 = mode;
    packet.param2 = key_id;
    memcpy(&packet.data[0], signature, ATECC_SIG_SIZE);
    if (verify_mode == VERIFY_MODE_EXTERNAL) {
        memcpy(&packet.data[ATECC_SIG_SIZE], public_key, ATECC_PUB_KEY_SIZE);
    } else if (other_data) {
        memcpy(&packet.data[ATECC_SIG_SIZE], other_data, VERIFY_OTHER_DATA_SIZE);
    }

    ATECC_COMMAND command = atecc_device_ref()->mCommands;
    if (atecc_command_verify(command, &packet) == false) {
        return false;
    }
    if (atecc_command_execute(&packet, atecc_device_ref()) == false) {
        return false;
    }

    // The Verify command may return MAC if requested
    if ((mac != NULL) && (packet.data[ATECC_IDX_COUNT] >= (ATECC_PACKET_OVERHEAD + MAC_SIZE))) {
        memcpy(mac, &packet.data[ATECC_IDX_RSP_DATA], MAC_SIZE);
    }

    // verified
    return true;
}

bool atecc_verify_extern(const uint8_t *message, const uint8_t *signature, const uint8_t *public_key)
{
    if (signature == NULL || message == NULL || public_key == NULL) {
        fido_log_error("atecc_verify_extern failed: BAD_PARAM");
        return false;
    }

    // Load message into device
    uint8_t nonce_target = NONCE_MODE_TARGET_TEMPKEY;
    uint8_t verify_source = VERIFY_MODE_SOURCE_TEMPKEY;
    if (atecc_device_ref()->mCommands->dt == ATECC608A) {
        // Use the Message Digest Buffer for the ATECC608A
        nonce_target = NONCE_MODE_TARGET_MSGDIGBUF;
        verify_source = VERIFY_MODE_SOURCE_MSGDIGBUF;
    }
    if (atecc_nonce_load(nonce_target, message, 32) == false) {
        return false;
    }

    if (atecc_verify(VERIFY_MODE_EXTERNAL | verify_source, VERIFY_KEY_P256, signature, public_key, NULL, NULL) == false) {
        return false;
    }

    return true;
}
