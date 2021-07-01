/* 
 * File:   AppHIDCommand.cpp
 * Author: makmorit
 *
 * Created on 2021/07/01, 11:30
 */
#include <zephyr.h>
#include <logging/log.h>

LOG_MODULE_DECLARE(AppHIDCommand);

#include "AppProcess.h"
#include "AppUSB.h"

//
// HIDコマンド関連
//
static uint8_t m_request[64];
static uint8_t m_response[64];
static uint8_t m_cid[4];

static void GenerateNewCID(void)
{
    // id: INITコマンド受信ごとに増加
    static uint8_t id = 1;

    // CID 01 00 33 nn
    m_cid[0] = 1;
    m_cid[1] = 0;
    m_cid[2] = 0x33;
    m_cid[3] = id++;
}

static void GenerateInitResponse(void)
{
    // CTAP_HID_INIT に対するレスポンスを生成
    uint8_t offset = 6;
    memset(m_response, 0, sizeof(m_response));
    memcpy(m_response, m_request, offset);

    // bytes
    m_response[offset++] = 17;

    // nonce
    memcpy(m_response + offset, m_request + offset, 8);
    offset += 8;

    // CID 01 00 33 nn
    GenerateNewCID();
    memcpy(m_response + offset, m_cid, sizeof(m_cid));
    offset += sizeof(m_cid);

    // others
    m_response[offset++] = 2;
    m_response[offset++] = 1;
    m_response[offset++] = 1;
    m_response[offset++] = 0;
    m_response[offset++] = 0;
}

static void GenerateCommandResponse(uint8_t status)
{
    uint8_t offset = 6;
    memset(m_response, 0, sizeof(m_response));
    memcpy(m_response, m_request, offset);

    // bytes
    m_response[offset++] = 1;

    // status
    m_response[offset++] = status;
}

void AppHIDCommandReceived(uint8_t *data, size_t size)
{
    // 受信フレームのデータを一時領域にコピー
    memcpy(m_request, data, size);

    uint8_t command = m_request[4];
    if (m_request[0] == 0xff && command == 0x86) {
        // CTAP_HID_INIT に対してレスポンスを実行
        GenerateInitResponse();
        AppUSBHidSendReport(m_response, sizeof(m_response));

    } else if (command & 0x80) {
        // CIDのチェック
        uint8_t status = 0;
        if (memcmp(m_cid, m_request, sizeof(m_cid)) == 0) {
            // 正しいCIDの場合、コマンドに対応する処理を実行
            AppProcessUSBHIDCommand(command);

        } else {
            // CTAP1_ERR_INVALID_CHANNEL を戻す
            LOG_ERR("HID channel is invalid");
            status = 0x0b;
        }
        
        // レスポンスを実行
        GenerateCommandResponse(status);
        AppUSBHidSendReport(m_response, sizeof(m_response));
    }
}
