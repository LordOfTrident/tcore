/* gethostname */
#define _POSIX_C_SOURCE 200112L
/* sethostname */
#define _DEFAULT_SOURCE

#include <stdio.h> /* printf, puts */
#include <string.h> /* strcmp */
#include <unistd.h> /* gethostname */
#include <stdlib.h> /* EXIT_SUCCESS */
#include <limits.h> /* HOST_NAME_MAX */
#include <errno.h> /* strerror, errno */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "hostname"
#define PROGRAM_DESC "Set/get the current hostname."

const char *g_usages[] = {
	"[NEW_HOSTNAME]"
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
	ERR_WATCH_INIT;

	if (p_argc == 2) {
		if (sethostname(p_argv[1], strlen(p_argv[1])) != 0) {
			ERR_SET_G_ERROR("sethostname", strerror(errno), ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			return EXIT_FAILURE;
		}
	} else if (p_argc == 1) {
		char hostname[HOST_NAME_MAX] = {0};
		if (gethostname(hostname, HOST_NAME_MAX - 1) != 0) {
			ERR_SET_G_ERROR("gethostname fail", strerror(errno), ERR_FATAL);
			error_fatal(PROGRAM_NAME);
		}
		puts(hostname);
	} else {
		ERR_SET_G_ERROR(p_argv[2], "Extra argument", ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
