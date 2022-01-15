#include <stdio.h> /* printf, puts, getchar */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdbool.h> /* true */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "no"
#define PROGRAM_DESC "Repeatedly output a line from arguments or standard input."

const char *g_usages[] = {
	"[LINE...]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"}
};

char *mgetline(size_t p_chunk_size) {
	size_t len = 0;
	char *buf = emalloc(p_chunk_size);
	if (buf == NULL)
		return NULL;

	char ch;
	while ((ch = getchar()) != '\n') {
		if (len + 1 > p_chunk_size) {
			p_chunk_size *= 2;
			void *tmp = erealloc(buf, p_chunk_size);
			if (tmp == NULL) {
				free(buf);
				return NULL;
			}
			buf = tmp;
		}

		buf[len ++] = ch;
	}
	if (len + 1 > p_chunk_size) {
		void *tmp = erealloc(buf, p_chunk_size + 1);
		if (tmp == NULL) {
			free(buf);
			return NULL;
		}
		buf = tmp;
	}

	buf[len] = '\0';
	return buf;
}

int main(int p_argc, const char **p_argv) {
	if (p_argc == 2) {
		if (strcmp(p_argv[1], "--version") == 0) {
			version(PROGRAM_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
			return EXIT_SUCCESS;
		} else if (strcmp(p_argv[1], "--help") == 0) {
			if (
				help(
					PROGRAM_NAME,
					g_usages, sizeof(g_usages) / sizeof(const char*),
					PROGRAM_DESC,
					g_arg_desc, sizeof(g_arg_desc) / sizeof(t_arg_desc)
				) != EXIT_SUCCESS
			)
				error_fatal(PROGRAM_NAME);
			else
				return EXIT_SUCCESS;
		}
	}

	char *str;
	if (p_argc <= 1) {
		str = mgetline(4);
		if (str == NULL)
			error_fatal(PROGRAM_NAME);

		while (true)
			puts(str);

		free(str);
	} else {
		while (true) {
			for (const char **p = p_argv + 1; *p != NULL; ++ p) {
				fputs(*p, stdout);
				/* Dont output the last useless space, it
				   might bug the output                   */
				if (*(p + 1) != NULL)
					putchar(' ');
			}
			putchar('\n');
		}
	}

	return EXIT_SUCCESS;
}
