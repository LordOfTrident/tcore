#include <stdio.h> /* printf, fputs, putchar, fgets */
#include <string.h> /* strcmp */
#include <unistd.h> /* execvp */
#include <sys/types.h> /* pid_t */
#include <sys/wait.h> /* wait */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <errno.h> /* strerror, errno */
#include <ctype.h> /* tolower */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "askdo"
#define PROGRAM_DESC "Ask the user if they want to run the command."

const char *g_usages[] = {
	"COMMAND [ARGS...]"
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

	++ p_argv;
	-- p_argc;
	char **argv = (char**)p_argv;

	fputs(PROGRAM_NAME": Run '", stdout);
	for (; *p_argv != NULL; ++ p_argv) {
		fputs(*p_argv, stdout);
		if (*(p_argv + 1) != NULL)
			putchar(' ');
	}
	fputs("'? Y/N: ", stdout);

	char answer[2];
	if (fgets(answer, sizeof(answer), stdin) == NULL) {
		ERR_SET_G_ERROR("fgets fail", NULL, ERR_FATAL);
		error_fatal(PROGRAM_NAME);
	}

	if (tolower(*answer) != 'y')
		return EXIT_SUCCESS;

	int status;
	pid_t childpid = fork();
	if (childpid == 0) {
		if (execvp(argv[0], (char**)argv) == -1) {
			ERR_SET_G_ERROR("execvp fail", strerror(errno), ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			return EXIT_FAILURE;
		}
	} else if (childpid > 0)
		childpid = wait(&status);

	return EXIT_SUCCESS;
}
