#include <stdio.h> /* printf, fprintf, puts */
#include <string.h> /* strcmp */
#include <time.h> /* localtime, strftime, tm */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "date"
#define PROGRAM_DESC "Print the current time and date in a C style format."

const char *usages[] = {
	"[+FORMAT]"
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

	const char *format = "%a %b %e %H:%M:%S %Z %Y";
	if (p_argc > 1 && p_argv[1][0] == '+') {
		++ p_argv;
		-- p_argc;

		format = &p_argv[0][1];
	}

	ERR_WATCH; if (p_argc > 1) {
		ERR_SET_G_ERROR(p_argv[1], "Extra argument", ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		return EXIT_FAILURE;
	}

	time_t tm = time(NULL);
	char buf[64] = {0};
	strftime(buf, sizeof(buf), format, localtime(&tm));
	puts(buf);

	return EXIT_SUCCESS;
}
