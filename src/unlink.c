#include <stdio.h> /* printf, putchar, size_t */
#include <string.h> /* strcmp */
#include <unistd.h> /* unlink */
#include <errno.h> /* errno, strerror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "unlink"
#define PROGRAM_DESC "Unlink a file."

const char *usages[] = {
	"[IGNORED_ARGS...]"
};

t_arg_desc arg_desc[] = {
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
					usages, sizeof(usages) / sizeof(const char*),
					PROGRAM_DESC,
					arg_desc, sizeof(arg_desc) / sizeof(t_arg_desc)
				) != EXIT_SUCCESS
			)
				error_fatal(PROGRAM_NAME);
			else
				return EXIT_SUCCESS;
		}
	}
	ERR_WATCH_INIT;

	int exitcode = EXIT_SUCCESS;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		ERR_WATCH; if (unlink(*p_argv) != 0) {
			ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			exitcode = EXIT_FAILURE;
		}
	}

	return exitcode;
}
