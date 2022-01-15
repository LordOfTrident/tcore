#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcmp */
#include <sys/stat.h> /* stat */
#include <utime.h> /* utimbuf, utime */
#include <time.h> /* time */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <errno.h> /* strerror, errno */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "touch"
#define PROGRAM_DESC "Update the access and modification times of a file. If no\n"\
                     "such file exists, a new empty file will be created."

const char *g_usages[] = {
	"FILE..."
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

	if (p_argc < 2) {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		try(PROGRAM_NAME);
		return EXIT_FAILURE;
	}

	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		struct stat entstat;
		struct utimbuf new_times;

		if (stat(*p_argv, &entstat) != 0) {
			FILE *fhnd = fopen(*p_argv, "w");
			if (fhnd == NULL) {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				return EXIT_FAILURE;
			}
			fclose(fhnd);

			if (stat(*p_argv, &entstat) != 0) {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_FATAL);
				error_fatal(PROGRAM_NAME);
			}
		}

		new_times.actime  = time(NULL);
		new_times.modtime = time(NULL);
		utime(*p_argv, &new_times);
	}

	return EXIT_SUCCESS;
}
