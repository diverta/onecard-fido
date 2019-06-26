#ifndef BLE_U2F_REGISTER_H__
#define BLE_U2F_REGISTER_H__

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_register_do_process(void);
void ble_u2f_register_send_response(void const *p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_REGISTER_H__

/** @} */
