#include <stdio.h> /* printf */
#include <string.h> /* strcmp, strspn */
#include <sys/stat.h> /* stat, mkdir */
#include <sys/types.h> /* mode_t */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <errno.h> /* strerror, errno, ENOENT */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "mkdir"
#define PROGRAM_DESC "Exit with an exit code indicating success."

const char *g_usages[] = {
	"[IGNORED_ARGS...]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"}
};

int check_create_folder(char *p_path, mode_t p_mode) {
	ERR_WATCH_INIT;
	struct stat entstat;
	ERR_WATCH; if (stat(p_path, &entstat) != 0) {
		if (errno == ENOENT) {
			ERR_WATCH; if (mkdir(p_path, p_mode) != 0) {
				ERR_SET_G_ERROR(p_path, strerror(errno), ERR_NOT_FATAL);
				return EXIT_FAILURE;
			}
		} else {
			ERR_SET_G_ERROR(p_path, strerror(errno), ERR_NOT_FATAL);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

int fix_path(const char *p_path, mode_t p_mode) {
	size_t len = strlen(p_path);

	/* if the entire path is just slashes, pass a single slash to mkdir */
	if (strspn(p_path, "/") == len)
		return check_create_folder("/", p_mode);

	char *orig = strclone(p_path);
	if (orig == NULL)
		return EXIT_FAILURE;

	char *tmp;
	char *path = orig;
	if (*path == '/') {
		while (path[1] == '/')
			++ path;
		tmp = path - 1;
	} else
		tmp = path;

	bool loop = true;
	while (loop) {
		while (*tmp != '/' && *tmp != '\0')
			++ tmp;
		if (*tmp == '\0')
			loop = false;
		*tmp = '\0';

		if (check_create_folder(path, p_mode) != EXIT_SUCCESS) {
			free(orig);
			return EXIT_FAILURE;
		}

		if (loop) {
			*tmp = '/';
			while (*tmp == '/')
				++ tmp;
		}
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
					usages, sizeof(g_usages) / sizeof(const char*),
					PROGRAM_DESC,
					arg_desc, sizeof(g_arg_desc) / sizeof(t_arg_desc)
				) != EXIT_SUCCESS
			)
				error_fatal(PROGRAM_NAME);
			else
				return EXIT_SUCCESS;
		}
	}
	ERR_WATCH_INIT;

	bool fix = false;
	int exitcode  = EXIT_SUCCESS;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (strcmp(*p_argv, "-p") == 0) {
			fix = true;
			continue;
		};

		if (fix) {
			ERR_WATCH; if (fix_path(*p_argv, 0777) != EXIT_SUCCESS) {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		} else {
			ERR_WATCH; if (mkdir(*p_argv, 0777) != 0) {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		}
	}

	return exitcode;
}
