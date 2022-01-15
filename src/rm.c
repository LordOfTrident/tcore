#include <stdio.h> /* printf, putchar, size_t */
#include <string.h> /* strcmp */
#include <unistd.h> /* unlink, rmdir */
#include <sys/stat.h> /* stat, S_ISDIR */
#include <dirent.h> /* dirent, opendir, readdir, closedir */
#include <errno.h> /* errno */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <stdbool.h> /* bool, true, false */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "rm"
#define PROGRAM_DESC "Remove a file or a directory. By default, rm cant remove\n"\
                     "directories, because recursive deletion is disabled."

const char *g_usages[] = {
	"[-r] FILE..."
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-r",        "Remove contents of directories recursively"}
};

int remove_dir(const char *p_path) {
	ERR_WATCH_INIT;
	struct dirent *dirent;
	ERR_WATCH; DIR *dir = opendir(p_path);
	if (dir == NULL) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE;
	}
	size_t path_len = strlen(p_path);

	while ((dirent = readdir(dir)) != NULL) {
		if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0)
			continue;

		size_t len = path_len + strlen(dirent->d_name) + 2;
		char *path = emalloc(len);
		if (path == NULL)
			return EXIT_FAILURE;

		struct stat entstat;
		snprintf(path, len, "%s/%s", p_path, dirent->d_name);
		if (stat(path, &entstat) == 0) {
			if (S_ISDIR(entstat.st_mode) != 0) {
				if (remove_dir(path) == EXIT_FAILURE)
					return EXIT_FAILURE;
			} else
				unlink(path);
		} else {
			ERR_SET_G_ERROR(path, strerror(errno), ERR_FATAL);
			return EXIT_FAILURE;
		}

		free(path);
	}

	closedir(dir);
	rmdir(p_path);
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

	int exitcode = EXIT_SUCCESS;
	bool recursive = false;
	bool options   = true;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (options) {
			if (strcmp(*p_argv, "-r") == 0) {
				recursive = true;
				continue;
			}
			options = false;
		}

		ERR_WATCH; if (unlink(*p_argv) != 0) {
			if (errno == EISDIR && recursive) {
				if (remove_dir(*p_argv) != EXIT_SUCCESS) {
					if (g_error.fatal)
						error_fatal(PROGRAM_NAME);
					else {
						error_simple(PROGRAM_NAME);
						error_cleanup();
						exitcode = EXIT_FAILURE;
					}
				}
			} else {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		}
	}

	return exitcode;
}
