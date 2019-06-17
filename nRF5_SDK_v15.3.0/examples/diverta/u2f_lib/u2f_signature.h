#ifndef U2F_SIGNATURE_H__
#define U2F_SIGNATURE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


uint8_t *u2f_signature_data_buffer(void);
size_t   u2f_signature_data_size(void);
void     u2f_signature_base_data_size_set(size_t size);
uint32_t u2f_signature_do_sign(uint8_t *private_key_be);
bool     u2f_signature_convert_to_asn1(void);


#ifdef __cplusplus
}
#endif

#endif // U2F_SIGNATURE_H__

/** @} */
