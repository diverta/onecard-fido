#ifndef BLE_U2F_AUTHENTICATE_H__
#define BLE_U2F_AUTHENTICATE_H__

#ifdef __cplusplus
extern "C" {
#endif


void ble_u2f_authenticate_do_process(void);
void ble_u2f_authenticate_resume_process(void);
void ble_u2f_authenticate_send_response(void const *p_evt);


#ifdef __cplusplus
}
#endif

#endif // BLE_U2F_AUTHENTICATE_H__

/** @} */
