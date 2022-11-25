/* 
 * File:   fido_flash_blp_auth_param.h
 * Author: makmorit
 *
 * Created on 2022/11/21, 15:16
 */
#ifndef FIDO_FLASH_BLP_AUTH_PARAM_H
#define FIDO_FLASH_BLP_AUTH_PARAM_H

#ifdef __cplusplus
extern "C" {
#endif

bool     fido_flash_blp_auth_param_read(void);
bool     fido_flash_blp_auth_param_write(uint8_t *p_uuid_string, uint32_t scan_sec, uint32_t scan_enable);
uint8_t *fido_flash_blp_auth_param_service_uuid_string(void);
uint32_t fido_flash_blp_auth_param_service_uuid_scan_sec(void);
uint32_t fido_flash_blp_auth_param_service_uuid_scan_enable(void);

#ifdef __cplusplus
}
#endif

#endif /* FIDO_FLASH_BLP_AUTH_PARAM_H */
