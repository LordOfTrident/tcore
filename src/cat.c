#include <stdio.h> /* printf, fputs, fprintf, stdout, size_t */
#include <string.h> /* strcmp */
#include <stdlib.h> /* malloc, free, EXIT_SUCCESS, EXIT_FAILURE */
#include <errno.h> /* errno, strerror */

#include "info.h"
#include "error.h"
#include "utils.h"

#define CHUNK_SIZE 4

#define PROGRAM_NAME "cat"
#define PROGRAM_DESC "Output a file into the standard output. If no files are\n"\
                     "specified, read standard input."

const char *g_usages[] = {
	"[FILE...]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"}
};

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

	int exitcode = EXIT_SUCCESS;
	if (p_argc < 2) {
		size_t len = 0;
		size_t chunk_size = CHUNK_SIZE;
		char *buf = emalloc(chunk_size);
		if (buf == NULL)
			error_fatal(PROGRAM_NAME);

		char ch;
		while ((ch = getchar()) != EOF) {
			++ len;
			if (len > chunk_size) {
				chunk_size *= 2;
				void *tmp = erealloc(buf, chunk_size);
				if (buf == NULL) {
					free(buf);
					error_fatal(PROGRAM_NAME);
				}
				buf = tmp;
			}

			buf[len - 1] = ch;
			if (ch == '\n') {
				buf[len] = '\0';
				fputs(buf, stdout);
				chunk_size = CHUNK_SIZE;
				len = 0;
				void *tmp = erealloc(buf, chunk_size);
				if (buf == NULL) {
					free(buf);
					error_fatal(PROGRAM_NAME);
				}
				buf = tmp;
			}
		}
		buf[len] = '\0';
		free(buf);
	} else {
		for (++ p_argv; *p_argv != NULL; ++ p_argv) {
			char *content = read_file(*p_argv);
			if (content == NULL) {
				if (g_error.fatal)
					error_fatal(PROGRAM_NAME);
				else {
					error_simple(PROGRAM_NAME);
					error_cleanup();
					try(PROGRAM_NAME);
					exitcode = EXIT_FAILURE;
				}
			} else {
				fputs(content, stdout);
				free(content);
			}
		}
	}

	return exitcode;
}
