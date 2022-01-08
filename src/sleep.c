/* eliminate missing nanosleep declaration warnings */
#define _POSIX_C_SOURCE	199309L

#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <time.h> /* nanosleep, timespec */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "sleep"
#define PROGRAM_DESC "Sleep for a given amount of time."

const char *usages[] = {
	"[-ms | -s | -m] NUM..."
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-ms",       "Set unit to miliseconds"},
	{"-s",        "Set unit to seconds"},
	{"-m",        "Set unit to minutes"}
};

typedef enum {
	unit_sec,
	unit_ms,
	unit_min
} t_unit;

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

	if (p_argc < 2) {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		try(PROGRAM_NAME);
		return EXIT_FAILURE;
	}

	int exitcode = EXIT_SUCCESS;
	t_unit unit  = unit_sec;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (strcmp(*p_argv, "-s") == 0) {
			unit = unit_sec;
			continue;
		} else if (strcmp(*p_argv, "-ms") == 0) {
			unit = unit_ms;
			continue;
		} else if (strcmp(*p_argv, "-min") == 0) {
			unit = unit_min;
			continue;
		}

		ERR_WATCH; if (strspn(*p_argv, "0123456789") != strlen(*p_argv)) {
			ERR_SET_G_ERROR(*p_argv, "Expected integer", ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			exitcode = EXIT_FAILURE;
			continue;
		}

		long amount = strtol(*p_argv, NULL, 10);
		struct timespec tsreq, tsrem;

		switch (unit) {
		case unit_ms:
			tsreq.tv_sec  = amount / 1000;
			tsreq.tv_nsec = (amount % 1000) * 1000000;
			break;
		case unit_sec:
			tsreq.tv_sec  = amount;
			tsreq.tv_nsec = 0;
			break;
		case unit_min:
			tsreq.tv_sec  = amount * 60;
			tsreq.tv_nsec = 0;
			break;
		}

		ERR_WATCH; if (nanosleep(&tsreq, &tsrem) != 0) {
			ERR_SET_G_ERROR("nanosleep fail", NULL, ERR_FATAL);
			error_fatal(PROGRAM_NAME);
		}
	}

	return exitcode;
}
