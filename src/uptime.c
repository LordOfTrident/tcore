#include <stdio.h> /* printf, puts */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS */
#include <sys/sysinfo.h> /* sysinfo */
#include <errno.h> /* strerror, errno */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "uptime"
#define PROGRAM_DESC "Output the current uptime in seconds."

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

	struct sysinfo info;
	ERR_WATCH; if (sysinfo(&info) != 0) {
		ERR_SET_G_ERROR("sysinfo fail", strerror(errno), ERR_FATAL);
		error_fatal(PROGRAM_NAME);
	}

	printf("%lus\n", info.uptime);

	return EXIT_SUCCESS;
}
