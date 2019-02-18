/* 
 * File:   ctap2_key_agreement.h
 * Author: makmorit
 *
 * Created on 2019/02/18, 15:31
 */
#ifndef CTAP2_KEY_AGREEMENT_H
#define CTAP2_KEY_AGREEMENT_H

#ifdef __cplusplus
extern "C" {
#endif

void    ctap2_key_agreement_generate_keypair(void);
uint8_t ctap2_key_agreement_encode_cose_key(void);

#ifdef __cplusplus
}
#endif

#endif /* CTAP2_KEY_AGREEMENT_H */

