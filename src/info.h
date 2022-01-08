#ifndef VERSION_H__HEADER_GUARD__
#define VERSION_H__HEADER_GUARD__

#include <stdio.h> /* printf, putchar */
#include <string.h> /* strlen, strchr */

#include "utils.h"

typedef struct {
	const char *arg;
	const char *desc;
} t_arg_desc;

static inline int help(
	const char *p_prog_name,
	const char *p_usages[],
	size_t p_usages_len,
	const char *p_desc,
	t_arg_desc p_args[],
	size_t p_args_len
) {
	if (p_usages != NULL && p_prog_name != NULL) {
		for (size_t i = 0; i < p_usages_len; ++ i) {
			if (i == 0)
				printf("usage: %s %s\n", p_prog_name, p_usages[i]);
			else
				printf("or:    %s %s\n", p_prog_name, p_usages[i]);
		}
		putchar('\n');
	}
	if (p_desc != NULL) {
		printf("%s\n\n", p_desc);
	}

	if (p_args != NULL) {
		size_t len = 0;
		char spaces[32] = {0};
		memset(spaces, ' ', sizeof(spaces) - 1);
		for (size_t i = 0; i < p_args_len; ++ i) {
			size_t arg_len = strlen(p_args[i].arg);
			if (arg_len > len)
				len = arg_len;
		}
		if (len > sizeof(spaces) - 1)
			len = sizeof(spaces) - 1;
		spaces[len] = '\0';

		for (size_t i = 0; i < p_args_len; ++ i) {
			size_t pos = len - strlen(p_args[i].arg);
			spaces[pos] = '\0';
			printf("%s%s  ", p_args[i].arg, spaces);
			spaces[pos] = ' ';

			char *buf = strclone(p_args[i].desc);
			if (buf == NULL)
				return EXIT_FAILURE;
			char *tmp = buf;
			char *out = buf;
			bool iterated = false;
			bool loop = true;
			while (loop) {
				tmp = strchr(tmp, '\n');
				if (tmp == NULL) {
					tmp = out;
					loop = false;
				} else
					*tmp = '\0';

				if (iterated)
					printf("%s  ", spaces);
				printf("%s\n", out);

				++ tmp;
				out = tmp;
				iterated = true;
			}
			free(buf);
		}
	}
	return EXIT_SUCCESS;
}

static inline void version(
	const char *p_prog_name,
	int p_ver_major,
	int p_ver_minor,
	int p_ver_patch
) {
	printf(
		"tcore %s version %i.%i.%i\n",
		p_prog_name,
		p_ver_major,
		p_ver_minor,
		p_ver_patch
	);
}

static inline void try(const char* p_prog_name) {
	printf("Try '%s --help' for usage\n", p_prog_name);
}

#endif /* VERSION_H__HEADER_GUARD__ */
