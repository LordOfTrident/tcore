#include <stdio.h> /* puts, size_t */
#include <string.h> /* strcmp, strspn, strrchr */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdbool.h> /* bool, true, false */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "basename"
#define PROGRAM_DESC "Print name without any directories.\nIf suffix is specified," \
                     "remove it too."

const char *usages[] = {
	"NAME [SUFFIX]",
	"[-s SUFFIX] -a NAME... | NAME",
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-s SUFFIX", "Suffix to remove"},
	{"-a",        "Multiple NAME arguments"}
};

char *basename(char *p_str, const char *p_suffix) {
	char *orig = strclone(p_str);
	if (orig == NULL)
		return NULL;
	char *buf = orig;

	/* if the entire path is just slashes, return a single slash */
	if (strspn(buf, "/") == strlen(p_str))
		return "/";

	char *tmp = buf;
	while (tmp[1] != '\0')
		++ tmp;

	if (*tmp == '/') {
		while (tmp >= buf && *tmp == '/')
			-- tmp;
		tmp[1] = '\0';
	}

	while (tmp >= buf && *tmp != '/')
		-- tmp;
	buf = tmp + 1;

	if (p_suffix != NULL) {
		char *name_suffix = buf + strlen(buf) - strlen(p_suffix);
		if (strcmp(name_suffix, p_suffix) == 0)
			name_suffix[0] = '\0';
	}

	char *ret = strclone(buf);
	free(orig);
	return ret;
}

int main(int p_argc, char **p_argv) {
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

	bool all = false;
	const char *suffix = NULL;

	if (strcmp(p_argv[1], "-s") == 0) {
		if (p_argc < 4) {
			ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			return EXIT_FAILURE;
		}

		/* move the arguments so it doesnt affect the loop */
		p_argv += 2;
		p_argc -= 2;
		suffix = p_argv[0];
	}
	if (strcmp(p_argv[1], "-a") == 0) {
		if (p_argc < 3) {
			ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			return EXIT_FAILURE;
		}

		all = true;
		++ p_argv;
		-- p_argc;
	}

	if (all) {
		for (++ p_argv; *p_argv != NULL; ++ p_argv) {
			*p_argv = basename(*p_argv, suffix);
			if (*p_argv == NULL)
				error_fatal(PROGRAM_NAME);
			puts(*p_argv);
		}
	} else {
		if (p_argc > 2) {
			if (suffix == NULL)
				suffix = p_argv[2];
			else {
				ERR_SET_G_ERROR(p_argv[2], "Extra argument", ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				return EXIT_FAILURE;
			}
		}

		p_argv[1] = basename(p_argv[1], suffix);
		if (p_argv[1] == NULL)
			error_fatal(PROGRAM_NAME);
		puts(p_argv[1]);
	}

	return EXIT_SUCCESS;
}
