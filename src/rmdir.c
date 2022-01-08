#include <stdio.h> /* printf, putchar, size_t */
#include <string.h> /* strcmp */
#include <unistd.h> /* rmdir */
#include <errno.h> /* errno, strerror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "rmdir"
#define PROGRAM_DESC "Remove non-empty directories."

const char *usages[] = {
	"[-p] DIR..."
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-p",        "Remove the directory an its ancestors"}
};

int delete_path(const char *p_path) {
	ERR_WATCH_INIT;
	size_t len = strlen(p_path);

	/* if the entire path is just slashes, pass a single slash to mkdir */
	if (strspn(p_path, "/") == len) {
		if (rmdir("/") != 0) {
			ERR_SET_G_ERROR("/", strerror(errno), ERR_NOT_FATAL);
			return EXIT_FAILURE;
		}
	}

	char *orig = strclone(p_path);
	if (orig == NULL)
		return EXIT_FAILURE;

	char *tmp = orig;
	while (tmp[1] != '\0')
		++ tmp;

	bool iterated = false;
	while (tmp >= orig && *tmp == '/') {
		-- tmp;
		iterated = true;
	}
	if (iterated)
		tmp[1] = '\0';

	tmp = orig;
	if (*tmp == '/') {
		while (tmp[1] == '/' && *tmp != '\0')
			++ tmp;
	}

	char *path = tmp;
	bool loop = true;
	while (loop) {
		if (rmdir(path) != 0) {
			ERR_SET_G_ERROR(path, strerror(errno), ERR_NOT_FATAL);
			free(orig);
			return EXIT_FAILURE;
		}

		tmp = strrchr(path, '/');
		if (tmp != NULL) {
			while (*tmp == '/')
				-- tmp;
			++ tmp;
			*tmp = '\0';
		} else
			loop = false;
	}

	free(orig);
	return EXIT_SUCCESS;
}

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

	bool delete_entire_path = false;
	int exitcode = EXIT_SUCCESS;
	bool options = true;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (options) {
			if (strcmp(*p_argv, "-p") == 0) {
				delete_entire_path = true;
				continue;
			}
			options = false;
		}

		if (delete_entire_path) {
			if (delete_path(*p_argv) != EXIT_SUCCESS) {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		} else {
			ERR_WATCH; if (rmdir(*p_argv) != 0) {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		}
	}

	return exitcode;
}
