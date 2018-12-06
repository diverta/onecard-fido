#ifndef U2F_REGISTER_H__
#define U2F_REGISTER_H__

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


uint16_t u2f_register_status_word(void);
void u2f_register_generate_keyhandle(uint8_t *p_appid_hash);
bool u2f_register_add_token_counter(uint8_t *p_appid_hash);
bool u2f_register_response_message(uint8_t *request_buffer, uint8_t *response_buffer, size_t *response_length, uint32_t apdu_le);


#ifdef __cplusplus
}
#endif

#endif // U2F_REGISTER_H__

/** @} */
