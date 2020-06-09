/* 
 * File:   usbd_service_ccid.h
 * Author: makmorit
 *
 * Created on 2020/04/27, 11:04
 */
#ifndef USBD_SERVICE_CCID_H
#define USBD_SERVICE_CCID_H

#ifdef __cplusplus
extern "C" {
#endif

//
// 関数群
//
void usbd_ccid_init(void);
void usbd_ccid_send_data_frame(uint8_t *p_data, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* USBD_SERVICE_CCID_H */
