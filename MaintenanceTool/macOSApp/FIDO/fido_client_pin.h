//
//  fido_client_pin.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2023/04/06.
//
#ifndef fido_client_pin_h
#define fido_client_pin_h

#include <stdlib.h>

bool        fido_client_pin_generate_pinauth(char *new_pin, char *old_pin, bool change_pin);
bool        fido_client_pin_generate_pinauth_from_pintoken(uint8_t *pin_token);
bool        fido_client_pin_generate_salt_auth(uint8_t *hmac_secret_salt);

#endif /* fido_client_pin_h */
