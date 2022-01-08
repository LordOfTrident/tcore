#define _POSIX_C_SOURCE	199309L

#include <stdio.h> /* printf, fprintf */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/stat.h> /* stat, S_ISDIR */
#include <time.h> /* nanosleep, timespec */
#include <errno.h> /* errono, strerror */

#include "error.h"

#define PROGRAM_NAME "stat"

void get_stat_date(char *p_buf, size_t p_size, time_t *p_sec) {
	strftime(p_buf, p_size, "%y-%m-%d", localtime(p_sec));
}

int main(int p_argc, const char **p_argv) {
	if (p_argc == 2 && strcmp(p_argv[1], "-v") == 0) {
		printf(
			"tcore %s version %i.%i.%i\n",
			PROGRAM_NAME,
			VERSION_MAJOR,
			VERSION_MINOR,
			VERSION_PATCH
		);
		return EXIT_SUCCESS;
	}
	ERR_WATCH_INIT;

	int exitcode = EXIT_SUCCESS;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		struct stat entstat;
		 ERR_WATCH; if (stat(*p_argv, &entstat) == 0) {
			char *type;
			char modif[32];
			char access[32];
			char change[32];
			if (S_ISDIR(entstat.st_mode) != 0)
				type = "directory";
			else
				type = "regular file";

			get_stat_date(modif,  sizeof(modif),  &entstat.st_atime);
			get_stat_date(access, sizeof(access), &entstat.st_mtime);
			get_stat_date(change, sizeof(change), &entstat.st_ctime);

			printf(
				"File:   %s\n"
				"Type:   %s\n"
				"Size:   %li bytes\n"
				"Access: %s\n"
				"Modif:  %s\n"
				"Change: %s\n",
				*p_argv,
				type,
				entstat.st_size,
				access,
				modif,
				change
			);
		} else {
			ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			exitcode = EXIT_FAILURE;
		}
	}

	return exitcode;
}
