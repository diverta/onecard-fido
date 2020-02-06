/* 
 * File:   fido_maintenance_cryption.h
 * Author: makmorit
 *
 * Created on 2019/12/09, 12:01
 */
#ifndef FIDO_MAINTENANCE_CRYPTION_H
#define FIDO_MAINTENANCE_CRYPTION_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *fido_maintenance_cryption_data(void);
size_t   fido_maintenance_cryption_size(void);
bool     fido_maintenance_cryption_restore(uint8_t *cbor_data_buffer, size_t cbor_data_length);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_MAINTENANCE_CRYPTION_H */
