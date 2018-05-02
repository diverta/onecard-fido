#include "sdk_common.h"
#if NRF_MODULE_ENABLED(BLE_U2F)
#include <stdio.h>
#include <string.h>
#include "ble_u2f.h"
#include "ble_u2f_crypto.h"
#include "ble_u2f_pairing.h"

// for pm_lesc_public_key_set
#include "peer_manager.h"

// for nrf_crypto functions
#include "nrf_crypto.h"
#include "nrf_crypto_keys.h"

// for logging informations
#define NRF_LOG_MODULE_NAME "ble_u2f_pairing_lesc"
#include "nrf_log.h"

static nrf_value_length_t m_peer_public_key_raw = {0};
__ALIGN(4) static ble_gap_lesc_p256_pk_t m_lesc_public_key;
__ALIGN(4) static ble_gap_lesc_dhkey_t   m_lesc_dh_key;

NRF_CRYPTO_ECC_PRIVATE_KEY_CREATE(m_private_key, SECP256R1);
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(m_public_key, SECP256R1);
NRF_CRYPTO_ECC_PUBLIC_KEY_CREATE(m_peer_public_key, SECP256R1);
NRF_CRYPTO_ECC_PUBLIC_KEY_RAW_CREATE_FROM_ARRAY(m_public_key_raw, SECP256R1, m_lesc_public_key.pk);
NRF_CRYPTO_ECDH_SHARED_SECRET_CREATE_FROM_ARRAY(m_dh_key, SECP256R1, m_lesc_dh_key.key);


uint32_t ble_u2f_pairing_lesc_generate_key_pair(void)
{
    // 暗号化モジュールを初期化
    ble_u2f_crypto_init();

    // キーペア(秘密鍵、公開鍵)を生成
    ret_code_t err_code = nrf_crypto_ecc_key_pair_generate(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &m_private_key, &m_public_key);
    APP_ERROR_CHECK(err_code);

    // 生成した公開鍵をバイナリー形式に変換
    err_code = nrf_crypto_ecc_public_key_to_raw(NRF_CRYPTO_BLE_ECDH_CURVE_INFO, &m_public_key, &m_public_key_raw);
    APP_ERROR_CHECK(err_code);

    // 鍵交換で使用する公開鍵をPeer Managerに引き渡す
    //  LESCペアリング要求時点で、Peer Managerから
    //  U2Fクライアントへ、この公開鍵が転送される
    err_code = pm_lesc_public_key_set(&m_lesc_public_key);
    APP_ERROR_CHECK(err_code);
    return err_code;
}

bool ble_u2f_pairing_lesc_on_ble_evt(ble_u2f_t *p_u2f, ble_evt_t * p_ble_evt)
{
    bool ret = true;
    ret_code_t err_code;
    uint8_t tmp0, tmp1;

    switch (p_ble_evt->header.evt_id) {
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // LESCペアリングが要求されたかどうかを確認
            tmp0 = p_ble_evt->evt.gap_evt.params.sec_params_request.peer_params.lesc;
            NRF_LOG_INFO("Security parameters requested: LESC=%d \r\n", tmp0);
            break;

        case BLE_GAP_EVT_LESC_DHKEY_REQUEST:
            // U2Fクライアントから公開鍵を取得
            m_peer_public_key_raw.p_value = &p_ble_evt->evt.gap_evt.params.lesc_dhkey_request.p_pk_peer->pk[0];
            m_peer_public_key_raw.length = BLE_GAP_LESC_P256_PK_LEN;

            // U2Fクライアントから取得した公開鍵をバイナリーに変換
            err_code = nrf_crypto_ecc_public_key_from_raw(NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
                                                          &m_peer_public_key_raw,
                                                          &m_peer_public_key);
            APP_ERROR_CHECK(err_code);

            // U2Fトークンで保持している秘密鍵と、
            // U2Fクライアントの公開鍵を利用し、共通鍵を生成
            err_code = nrf_crypto_ecdh_shared_secret_compute(NRF_CRYPTO_BLE_ECDH_CURVE_INFO,
                                                            &m_private_key,
                                                            &m_peer_public_key,
                                                            &m_dh_key);
            APP_ERROR_CHECK(err_code);

            // 生成された共通鍵をU2Fクライアントに戻す
            err_code = sd_ble_gap_lesc_dhkey_reply(p_ble_evt->evt.gap_evt.conn_handle, &m_lesc_dh_key);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_INFO("LE Secure Connection dhkey reply success \r\n");
            break;

        case BLE_GAP_EVT_CONN_SEC_UPDATE:
            tmp0 = p_ble_evt->evt.gap_evt.params.conn_sec_update.conn_sec.sec_mode.sm;
            tmp1 = p_ble_evt->evt.gap_evt.params.conn_sec_update.conn_sec.sec_mode.lv;
            NRF_LOG_INFO("Security connection updated: Security Mode=%d, Level=%d \r\n", tmp0, tmp1);
            break;

        case BLE_GAP_EVT_AUTH_STATUS:
            // ペアリングが成功したかどうかを判定
            ble_u2f_pairing_on_evt_auth_status(p_u2f, p_ble_evt);
            break;

        default:
            ret = false;
            break;
    }

    return ret;
}

#endif // NRF_MODULE_ENABLED(BLE_U2F)
