#include <stdio.h> /* printf, fprintf, puts */
#include <string.h> /* strcmp */
#include <unistd.h> /* getcwd */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"

#define PATH_MAX 4096

#define PROGRAM_NAME "pwd"
#define PROGRAM_DESC "Output the path of the current working directory."

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

	char buf[PATH_MAX] = {0};
	ERR_WATCH; if (getcwd(buf, sizeof(buf)) == NULL) {
		ERR_SET_G_ERROR("getcwd fail", NULL, ERR_FATAL);
		error_fatal(PROGRAM_NAME);
	}
	puts(buf);

	return EXIT_SUCCESS;
}
