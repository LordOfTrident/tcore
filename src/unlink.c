#include <stdio.h> /* printf, putchar, size_t */
#include <string.h> /* strcmp */
#include <unistd.h> /* unlink */
#include <errno.h> /* errno, strerror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "error.h"

#define PROGRAM_NAME "unlink"

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
		ERR_WATCH; if (unlink(*p_argv) != 0) {
			ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			exitcode = EXIT_FAILURE;
		}
	}

	return exitcode;
}
