//
//  tool_piv_admin.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2020/12/03.
//
#ifndef tool_piv_admin_h
#define tool_piv_admin_h

#include <stdio.h>

#define TAG_CERT                    0x70
#define TAG_CERT_COMPRESS           0x71
#define TAG_CERT_LRC                0xfe

#define TAG_DATA_OBJECT             0x5c
#define TAG_DATA_OBJECT_VALUE       0x53

//
// public functions
//
unsigned char   *tool_piv_admin_des_default_key(void);

bool             tool_piv_admin_load_private_key(unsigned char key_slot_id, const char *pem_path, unsigned char *algorithm);
bool             tool_piv_admin_load_certificate(unsigned char key_slot_id, const char *pem_path);
unsigned char   *tool_piv_admin_generated_APDU_data(void);
size_t           tool_piv_admin_generated_APDU_size(void);
unsigned char   *tool_piv_admin_generate_CHUID_APDU(size_t *size);
unsigned char   *tool_piv_admin_generate_CCC_APDU(size_t *size);
size_t           tool_piv_admin_set_object_header(unsigned int object_id, unsigned char *buffer);

bool             tool_piv_admin_extract_cert_from_TLV(unsigned char *buffer, size_t size);
unsigned char   *tool_piv_admin_extracted_cert_data(void);
size_t           tool_piv_admin_extracted_cert_size(void);

#endif /* tool_piv_admin_h */
