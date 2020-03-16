/* 
 * File:   fido_flash_skey_cert.h
 * Author: makmorit
 *
 * Created on 2019/07/09, 10:21
 */
#ifndef FIDO_FLASH_SKEY_CERT_H
#define FIDO_FLASH_SKEY_CERT_H

#ifdef __cplusplus
extern "C" {
#endif

uint8_t *fido_flash_cert_data(void);
uint32_t fido_flash_cert_data_length(void);
bool     fido_flash_skey_cert_delete(void);
bool     fido_flash_skey_cert_read(void);
bool     fido_flash_skey_cert_available(void);
bool     fido_flash_skey_cert_data_prepare(uint8_t *data, uint16_t length);
bool     fido_flash_skey_cert_write(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_SKEY_CERT_H */
