#ifndef BSEARCH_LEFT_H
#define BSEARCH_LEFT_H

#include <stdlib.h>

/* documented in the C file */
size_t bsearch_left(const void *key, const void *base, size_t num, size_t size,
               int (*cmp)(const void *key, const void *elt));

#endif
