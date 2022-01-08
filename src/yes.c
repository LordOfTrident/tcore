#include <stdio.h> /* printf, puts */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdbool.h> /* true */

#include "info.h"

#define PROGRAM_NAME "yes"
#define PROGRAM_DESC "Repeatedly output a line from arguments or 'y'."

const char *usages[] = {
	"[LINE...]"
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

	const char *str = "y";
	while (true) {
		if (p_argc <= 1)
			puts(str);
		else {
			for (const char **p = p_argv + 1; *p != NULL; ++ p) {
				fputs(*p, stdout);
				/* Dont output the last useless space, it
				   might bug the output                   */
				if (*(p + 1) != NULL)
					putchar(' ');
			}
			putchar('\n');
		}
	}

	return EXIT_SUCCESS;
}
