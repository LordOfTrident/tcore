#include <stdio.h> /* printf, fprintf, puts, putchar, fputs, stdout */
#include <string.h> /* strcmp, strchr */
#include <dirent.h> /* dirent, opendir, readdir, closedir */
#include <sys/stat.h> /* stat, S_ISDIR */
#include <sys/types.h> /* mode_t */
#include <time.h> /* nanosleep, timespec */
#include <stdbool.h> /* bool, true, false */
#include <errno.h> /* errono, strerror */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"
#include "utils.h"

/* modes */
#define MODE_EXEC  01
#define MODE_WRITE 02
#define MODE_READ  04

/* offsets */
#define BOF_OTHER 0
#define BOF_GROUP 3
#define BOF_USER  6

/* flags */
#define LIST_DIR_SHOW_ALL  0x1
#define LIST_DIR_LONG_LIST 0x2
#define LIST_DIR_RECURSIVE 0x4

#define PROGRAM_NAME "ls"
#define PROGRAM_DESC "List entries in the current directory."

const char *g_usages[] = {
	"[-a | -l] [DIR...]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-a",        "Dont ignore entries starting with a dot"},
	{"-l",        "Long listing format, output: perms, type, modif date, size,\n"
	              "user"},
	{"-r",        "Recursively list subdirectories"}
};

void get_stat_date_time(char *p_buf, size_t p_size, time_t *p_sec) {
	strftime(p_buf, p_size, "%c", localtime(p_sec));
}

void write_perms(char *p_buf, mode_t p_mode, int p_off) {
	if (p_mode & (MODE_READ << p_off))
		p_buf[8 - (p_off + 2)] = 'r';
	if (p_mode & (MODE_WRITE << p_off))
		p_buf[8 - (p_off + 1)] = 'w';
	if (p_mode & (MODE_EXEC << p_off))
		p_buf[8 - p_off] = 'x';
}

int list_dir(const char *p_path, int p_flags) {
	ERR_WATCH_INIT;
	struct dirent *dirent;
	ERR_WATCH; DIR *dir = opendir(p_path);
	if (dir == NULL) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE;
	}

	char **dirs = NULL;
	size_t dirs_len = 0;
	bool recursive = p_flags & LIST_DIR_RECURSIVE;
	bool long_list = p_flags & LIST_DIR_LONG_LIST;
	if (recursive) {
		dirs = (char**)emalloc(1);
		if (dirs == NULL)
			return EXIT_FAILURE;
	}
	while ((dirent = readdir(dir)) != NULL) {
		if (dirent->d_name[0] != '.' || p_flags & LIST_DIR_SHOW_ALL) {
			char *path = (char*)emalloc(strlen(p_path) + strlen(dirent->d_name) + 2);
			if (path == NULL)
				return EXIT_FAILURE;
			strcpy(path, p_path);
			strcat(path, "/");
			strcat(path, dirent->d_name);

			struct stat entstat;
			ERR_WATCH; if (stat(path, &entstat) != 0) {
				ERR_SET_G_ERROR(path, strerror(errno), ERR_FATAL);
				free(path);
				return EXIT_FAILURE;
			}
			bool is_dir = S_ISDIR(entstat.st_mode);

			char tmp[3] = {0};
			if (long_list) {
				char *type;
				char modif[32];
				char perms[11] = "---------";
				if (is_dir)
					type = "d";
				else
					type = "f";

				get_stat_date_time(modif, sizeof(modif), &entstat.st_atime);

				write_perms(perms, entstat.st_mode, BOF_OTHER);
				write_perms(perms, entstat.st_mode, BOF_GROUP);
				write_perms(perms, entstat.st_mode, BOF_USER);

				printf("%s, %s, %s, %liB, ", perms, type, modif, entstat.st_size);

				tmp[0] = '\n';
			} else
				strcpy(tmp, "  ");

			if (strchr(dirent->d_name, ' ') != NULL) {
				if (is_dir)
					printf("'%s/'%s", dirent->d_name, tmp);
				else
					printf("'%s'%s", dirent->d_name, tmp);
			} else {
				if (is_dir)
					printf("%s/%s", dirent->d_name, tmp);
				else
					printf("%s%s", dirent->d_name, tmp);
			}

			if (
				recursive &&
				strcmp(dirent->d_name, "..") != 0 &&
				strcmp(dirent->d_name, ".") != 0 &&
				is_dir
			) {
				++ dirs_len;
				void *tmp = erealloc(dirs, sizeof(char*) * dirs_len);
				if (tmp == NULL) {
					free(dirs);
					free(path);
					return EXIT_FAILURE;
				}
				dirs = tmp;
				dirs[dirs_len - 1] = path;
			} else
				free(path);
		}
	}
	if (!long_list)
		putchar('\n');

	if (recursive) {
		bool error = false;
		for (size_t i = 0; i < dirs_len; ++ i) {
			printf("\n%s:\n", dirs[i]);
			if (list_dir(dirs[i], p_flags) != EXIT_SUCCESS)
				error = true;

			free(dirs[i]);
		}
		free(dirs);
		if (error)
			return EXIT_FAILURE;
	}

	closedir(dir);
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

	int exitcode = EXIT_SUCCESS;
	bool show_hidden = false;
	bool show_this_dir = true;
	bool long_list = false;
	bool options   = true;
	bool recursive = false;
	int flags = 0;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (options) {
			if (strcmp(*p_argv, "-a") == 0) {
				show_hidden = true;
				continue;
			} else if (strcmp(*p_argv, "-l") == 0) {
				long_list = true;
				continue;
			} else if (strcmp(*p_argv, "-r") == 0) {
				recursive = true;
				continue;
			}
			if (show_hidden)
				flags |= LIST_DIR_SHOW_ALL;
			if (long_list)
				flags |= LIST_DIR_LONG_LIST;
			if (recursive)
				flags |= LIST_DIR_RECURSIVE;

			options = false;
		}

		show_this_dir = false;
		if (list_dir(*p_argv, flags) != EXIT_SUCCESS) {
			if (g_error.fatal)
				error_fatal(PROGRAM_NAME);
			else {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				exitcode = EXIT_FAILURE;
			}
		}
	}

	/* if no directory was specified, output the files of the current one */
	if (show_this_dir) {
		if (show_hidden)
			flags |= LIST_DIR_SHOW_ALL;
		if (long_list)
			flags |= LIST_DIR_LONG_LIST;
		if (recursive)
			flags |= LIST_DIR_RECURSIVE;

		return list_dir(".", flags);
	}

	return exitcode;
}
