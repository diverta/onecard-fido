//
//  fido_blob.h
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#ifndef fido_blob_h
#define fido_blob_h

#include <stdio.h>

typedef struct fido_blob {
    unsigned char   *ptr;
    size_t           len;
} fido_blob_t;

// 関数群
void         explicit_bzero(void *p, size_t n);
fido_blob_t *fido_blob_new(void);
void         fido_blob_set(fido_blob_t *b, const unsigned char *ptr, size_t len);
void         fido_blob_free(fido_blob_t **bp);

#endif /* fido_blob_h */
