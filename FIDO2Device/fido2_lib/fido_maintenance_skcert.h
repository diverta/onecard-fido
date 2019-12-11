/* 
 * File:   fido_maintenance_skcert.h
 * Author: makmorit
 *
 * Created on 2019/12/09, 12:01
 */
#ifndef FIDO_MAINTENANCE_SKCERT_H
#define FIDO_MAINTENANCE_SKCERT_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *fido_maintenance_skcert_data(void);
size_t   fido_maintenance_skcert_size(void);
bool     fido_maintenance_skcert_restore(uint8_t *cbor_data_buffer, size_t cbor_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_SKCERT_H */
