#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdio.h> /* fputc, fputs, fseek, fopen, fclose, FILE, size_t */
#include <sys/stat.h> /* stat, S_ISDIR */
#include <string.h> /* strchr */
#include <stdlib.h> /* malloc, realloc, strtol */
#include <string.h> /* strcpy */
#include <errno.h> /* strerror, errno */

#include "error.h"

#define CHUNK_SIZE 4

void *emalloc(size_t p_bytes);
void *erealloc(void *p_buf, size_t p_bytes);
bool fputse(const char *p_str, FILE *p_stream);
char *read_file(const char *p_path);
int write_file(const char *p_path, const char *p_content, bool p_escaped);
char *strclone(const char *p_str); /* not apart of c99 standard */

#endif /* UTILS_H__HEADER_GUARD__ */
