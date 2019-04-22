//
//  fido_blob.c
//  MaintenanceTool
//
//  Created by Makoto Morita on 2019/04/22.
//
#include <string.h>
#include <stdlib.h>
#include "fido_blob.h"

void explicit_bzero(void *p, size_t n) {
    if (n == 0) return;
    memset(p, 0, n);
}

fido_blob_t *fido_blob_new(void) {
    return (fido_blob_t *)calloc(1UL, sizeof(fido_blob_t));
}

void fido_blob_set(fido_blob_t *b, const unsigned char *ptr, size_t len) {
    if (b->ptr != NULL) {
        explicit_bzero(b->ptr, b->len);
        free(b->ptr);
    }
    
    b->len = 0;
    b->ptr = malloc(len);
    
    if (b->ptr == NULL) {
        b->len = 0;
        return;
    }
    
    memcpy(b->ptr, ptr, len);
    b->len = len;
}

void fido_blob_free(fido_blob_t **bp) {
    fido_blob_t *b;
    
    if (bp == NULL || (b = *bp) == NULL)
        return;
    
    if (b->ptr) {
        explicit_bzero(b->ptr, b->len);
        free(b->ptr);
    }
    
    explicit_bzero(b, sizeof(*b));
    free(b);
    
    *bp = NULL;
}
