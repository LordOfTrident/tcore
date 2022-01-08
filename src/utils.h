#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdlib.h> /* malloc */
#include <string.h> /* strcpy */
#include <errno.h> /* strerror, errno */

#include "error.h"

void *emalloc(size_t p_bytes);
void *erealloc(void *p_buf, size_t p_bytes);
char *strclone(const char *p_str); /* not apart of c99 standard */

#endif /* UTILS_H__HEADER_GUARD__ */
