#ifndef U2F_AUTHENTICATE_H__
#define U2F_AUTHENTICATE_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

    
uint16_t u2f_authenticate_status_word(void);
bool     u2f_authenticate_restore_keyhandle(uint8_t *apdu_data);
uint8_t *u2f_authenticate_get_appid(uint8_t *apdu_data);
bool     u2f_authenticate_update_token_counter(uint8_t *p_appid_hash);
bool     u2f_authenticate_response_message(uint8_t *request_buffer, uint8_t *response_buffer, size_t *response_length, uint32_t apdu_le);


#ifdef __cplusplus
}
#endif

#endif // U2F_AUTHENTICATE_H__

/** @} */
