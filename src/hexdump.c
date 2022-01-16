#include <stdio.h> /* printf, putchar, fprintf, fopen, fclose, FILE */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/stat.h> /* stat, S_ISDIR */
#include <errno.h> /* errono, strerror */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "hexdump"
#define PROGRAM_DESC "Output file bytes in hexadecimal"

const char *g_usages[] = {
	"FILES..."
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"}
};

int hexdump(const char *p_path) {
	ERR_WATCH_INIT;
	struct stat entstat;
	if (stat(p_path, &entstat) == 0) {
		if (S_ISDIR(entstat.st_mode) != 0) {
			ERR_SET_G_ERROR(p_path, "Is a directory", ERR_NOT_FATAL);
			return EXIT_FAILURE;
		}
	} else {
		ERR_SET_G_ERROR("stat fail", strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE;
	}

	ERR_WATCH; FILE *fhnd = fopen(p_path, "r");
	if (fhnd == NULL) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_FATAL);
		return EXIT_FAILURE;
	}

	char ch;
	while ((ch = getc(fhnd)) != EOF)
		printf("%02x ", ch);
	putchar('\n');

	fclose(fhnd);
	return EXIT_SUCCESS;
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
	ERR_WATCH_INIT;

	if (p_argc < 2) {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		try(PROGRAM_NAME);
		return EXIT_FAILURE;
	}

	int exitcode = EXIT_SUCCESS;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (hexdump(*p_argv) != EXIT_SUCCESS) {
			if (g_error.fatal)
				error_fatal(PROGRAM_NAME);
			else {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		}
	}

	return exitcode;
}
