#include "utils.h"

void *emalloc(size_t p_bytes) {
	ERR_WATCH_INIT;
	void *buf = malloc(p_bytes);
	if (buf == NULL) {
		ERR_SET_G_ERROR("malloc fail", strerror(errno), ERR_FATAL);
		return NULL;
	}

	return buf;
}

void *erealloc(void *p_buf, size_t p_bytes) {
	ERR_WATCH_INIT;
	void *tmp = realloc(p_buf, p_bytes);
	if (tmp == NULL) {
		ERR_SET_G_ERROR("realloc fail", strerror(errno), ERR_FATAL);
		return NULL;
	}

	return tmp;
}

/* not apart of c99 standard */
char *strclone(const char *p_str) {
	char *buf = emalloc(strlen(p_str) + 1);
	if (buf == NULL)
		return NULL;

	strcpy(buf, p_str);
	return buf;
}
