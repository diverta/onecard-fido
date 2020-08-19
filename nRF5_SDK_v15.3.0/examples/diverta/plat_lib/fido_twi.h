/* 
 * File:   fido_twi.h
 * Author: makmorit
 *
 * Created on 2019/11/20, 11:10
 */
#ifndef FIDO_TWI_H
#define FIDO_TWI_H

#ifdef __cplusplus
extern "C" {
#endif

void const *fido_twi_instance_ref(void);
bool fido_twi_init(void);
bool fido_twi_write(uint8_t address, uint8_t *p_data, uint8_t length);
bool fido_twi_read(uint8_t address, uint8_t *p_data, uint8_t length);
bool fido_twi_verify_nack(uint8_t address);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_TWI_H */
