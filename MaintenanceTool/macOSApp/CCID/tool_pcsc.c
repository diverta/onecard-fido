//
//  tool_pcsc.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/09.
//
#include <stdlib.h>
#include <string.h>
#include <PCSC/winscard.h>

#include "debug_log.h"
#include "tool_pcsc.h"

// 共有情報を保持
static SCARDCONTEXT m_context;
static SCARDHANDLE  m_card;

// 接続先のSCardスロット名を保持
static const char *m_slot_name = "Diverta Inc. Secure Dongle";

// 送受信APDUを保持
static uint8_t  command_apdu[264];
static uint32_t command_apdu_size;
static uint8_t  response_apdu[264];
static uint32_t response_apdu_size;
static uint16_t response_sw;

const char *tool_pcsc_scard_slot_name(void)
{
    // 接続先のSCardスロット名を戻す
    return m_slot_name;
}

void tool_pcsc_scard_init(void)
{
    // 共有情報を初期化
    m_context = (SCARDCONTEXT)-1;
    m_card = 0;
}

bool tool_pcsc_scard_connect(void)
{
    // 共有情報の取得
    int32_t rc;
    if (SCardIsValidContext(m_context) != SCARD_S_SUCCESS) {
        rc = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &m_context);
        if (rc != SCARD_S_SUCCESS) {
            log_debug("SCardEstablishContext failed, rc=0x%08x", rc);
            return false;
        }
    }
    // CCID I/Fに接続（接続先：Diverta Inc. Secure Dongle で固定）
    uint32_t active_protocol;
    rc = SCardConnect(m_context, m_slot_name, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, &m_card, &active_protocol);
    if (rc != SCARD_S_SUCCESS) {
        log_debug("SCardConnect failed for '%s', rc=0x%08x", m_slot_name, rc);
        SCardReleaseContext(m_context);
        tool_pcsc_scard_init();
        return false;
    }
    // 正常終了
    return true;
}

bool tool_pcsc_scard_begin_transaction(void)
{
    // トランザクション開始
    int retries = 0;
    int32_t rc = SCardBeginTransaction(m_card);
    while (rc == SCARD_W_RESET_CARD && retries < 5) {
        retries++;
        uint32_t active_protocol = 0;
        rc = SCardReconnect(m_card, SCARD_SHARE_SHARED, SCARD_PROTOCOL_T1, SCARD_LEAVE_CARD, &active_protocol);
        if (rc != SCARD_S_SUCCESS) {
            log_debug("SCardReconnect failed for '%s', rc=0x%08x", m_slot_name, rc);
            return false;
        }
        rc = SCardBeginTransaction(m_card);
    }
    if (rc != SCARD_S_SUCCESS) {
        log_debug("SCardBeginTransaction failed for '%s' after %d retries, rc=0x%08x", m_slot_name, retries, rc);
        return false;
    }
    // 正常終了
    return true;
}

void tool_pcsc_scard_end_transaction(void)
{
    // 切断済みの場合は終了
    if (m_context == (SCARDCONTEXT)-1) {
        return;
    }
    // トランザクション終了
    int32_t rc = SCardEndTransaction(m_card, SCARD_LEAVE_CARD);
    if (rc != SCARD_S_SUCCESS) {
        log_debug("SCardEndTransaction failed for '%s', rc=0x%08x", m_slot_name, rc);
    }
}

void tool_pcsc_scard_disconnect(void)
{
    // 切断済みの場合は終了
    if (m_context == (SCARDCONTEXT)-1) {
        return;
    }
    // CCID I/Fから切断
    SCardDisconnect(m_card, SCARD_RESET_CARD);
    // 共有情報を解放
    if (SCardIsValidContext(m_context) == SCARD_S_SUCCESS) {
        SCardReleaseContext(m_context);
    }
    tool_pcsc_scard_init();
}

void tool_pcsc_scard_set_command_apdu(uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2, uint8_t lc, uint8_t *data, uint16_t le)
{
    // 送信APDUを生成
    command_apdu[0] = cla;
    command_apdu[1] = ins;
    command_apdu[2] = p1;
    command_apdu[3] = p2;
    command_apdu[4] = lc;
    memcpy(command_apdu + 5, data, lc);
    // 送信APDUサイズを設定
    command_apdu_size = lc + 5;
    // 受信APDUサイズを設定
    response_apdu_size = le;
}

bool tool_pcsc_scard_send_command_apdu(void)
{
    // データを送信（command_apduには、CLA/INS/P1/P2/Lcの５バイトを含むものとします）
    uint32_t size = response_apdu_size;
    int32_t rc = SCardTransmit(m_card, SCARD_PCI_T1, command_apdu, command_apdu_size, NULL, response_apdu, &size);
    if (rc != SCARD_S_SUCCESS) {
        log_debug("SCardTransmit failed for '%s', rc=0x%08x", m_slot_name, rc);
        response_sw = 0;
        response_apdu_size = 0;
        return false;
    }
    // ステータスワード、受信サイズ長（ステータスワードの２バイトを除く）を設定
    if (size >= 2) {
        response_sw = (response_apdu[size - 2] << 8) | response_apdu[size - 1];
        response_apdu_size = size - 2;
    } else {
        response_sw = 0;
        response_apdu_size = 0;
    }
    return true;
}

uint8_t *tool_pcsc_scard_response_apdu_data(size_t *apdu_data_size, uint16_t *sw)
{
    // 受信データサイズ、ステータスワード、受信データの参照を戻す
    *sw = response_sw;
    *apdu_data_size = (size_t)response_apdu_size;
    return response_apdu;
}
