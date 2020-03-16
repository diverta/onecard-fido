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
void     u2f_signature_generate_hash_for_sign(void);
uint8_t *u2f_signature_hash_for_sign(void);
bool     u2f_signature_convert_to_asn1(uint8_t *p_signature_value);


#ifdef __cplusplus
}
#endif

#endif // U2F_SIGNATURE_H__

/** @} */
