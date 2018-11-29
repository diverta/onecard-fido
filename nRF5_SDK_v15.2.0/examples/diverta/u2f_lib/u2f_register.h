#ifndef U2F_REGISTER_H__
#define U2F_REGISTER_H__

#include "fds.h"

#ifdef __cplusplus
extern "C" {
#endif


void u2f_register_generate_keyhandle(uint8_t *p_appid_hash);
bool u2f_register_add_token_counter(uint8_t *p_appid_hash);
bool u2f_register_response_message(uint8_t *request_buffer, uint8_t *response_buffer, size_t *response_length);


#ifdef __cplusplus
}
#endif

#endif // U2F_REGISTER_H__

/** @} */
