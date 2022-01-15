#include <stdio.h> /* printf, puts */
#include <string.h> /* strcmp */
#include <unistd.h> /* getlogin */
#include <stdlib.h> /* EXIT_SUCCESS */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "whoami"
#define PROGRAM_DESC "Print the current users name"

const char *g_usages[] = {
	"[IGNORED_ARGS...]"
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

	ERR_WATCH; char *user = getlogin();
	if (user == NULL) {
		ERR_SET_G_ERROR("getlogin fail", NULL, ERR_FATAL);
		error_fatal(PROGRAM_NAME);
	}
	puts(user);

	return EXIT_SUCCESS;
}
