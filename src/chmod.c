#include <stdio.h> /* printf */
#include <string.h> /* strcmp, strspn */
#include <sys/stat.h> /* chmod */
#include <sys/types.h> /* mode_t */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <errno.h> /* strerror, errno, ENOENT */

#include "info.h"
#include "error.h"
#include "utils.h"

/* modes */
#define MODE_EXEC  01
#define MODE_WRITE 02
#define MODE_READ  04

#define WHO_OTHER 01
#define WHO_GROUP 02
#define WHO_USER  04
#define WHO_ALL   07

/* actions */
#define ACTION_SET 1
#define ACTION_ADD 2
#define ACTION_SUB 3

/* offsets */
#define BOF_OTHER 0
#define BOF_GROUP 3
#define BOF_USER  6

#define PROGRAM_NAME "chmod"
#define PROGRAM_DESC "Change the mode of a file."

const char *usages[] = {
	"MODE FILE..."
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"}
};

/* this seems like the worst way to handle it, i might redo this function later */
int str_to_mode(const char *p_str, mode_t p_mode) {
	ERR_WATCH_INIT;
	const char *tmp = p_str;

	bool u, g, o;
	u = g = o = false;
	while (*tmp != '=' && *tmp != '+' && *tmp != '-' && *tmp != '\0') {
		switch (*tmp) {
		case 'a': u = g = o = true; break;
		case 'o': o = true;         break;
		case 'g': g = true;         break;
		case 'u': u = true;         break;
		default: goto l_invalid_mode;
		}
		++ tmp;
	}
	if (*tmp == '\0') {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		return EXIT_FAILURE_BELOWZERO;
	}
	if (!u && !g && !o)
		u = g = o = true;

	int action = ACTION_SET;
	bool r, w, x;
	r = w = x = false;
	switch (*tmp) {
	case '=': action = ACTION_SET; break;
	case '+': action = ACTION_ADD; break;
	case '-': action = ACTION_SUB; break;
	}

	while (*tmp != '\0') {
		switch (*tmp) {
		case '=': case '+': case '-': break;
		case 'r': r = true; break;
		case 'w': w = true; break;
		case 'x': x = true; break;
		default:
		l_invalid_mode:
			ERR_SET_G_ERROR(p_str, "Invalid mode", ERR_NOT_FATAL);
			return EXIT_FAILURE_BELOWZERO;
		}
		++ tmp;
	}

#define CHECK_MODE_SET(p_who, p_off) \
	if (p_who) { \
		p_mode &= ~(7 << BOF_USER); \
		if (r) \
			p_mode |= MODE_READ << p_off; \
		if (w) \
			p_mode |= MODE_WRITE << p_off; \
		if (x) \
			p_mode |= MODE_EXEC << p_off; \
	}

#define CHECK_MODE_ADD(p_who, p_off) \
	if (p_who) { \
		if (r) \
			p_mode |= MODE_READ << p_off; \
		if (w) \
			p_mode |= MODE_WRITE << p_off; \
		if (x) \
			p_mode |= MODE_EXEC << p_off; \
	}

#define CHECK_MODE_SUB(p_who, p_off) \
	if (p_who) { \
		if (r) \
			p_mode &= ~(MODE_READ << p_off); \
		if (w) \
			p_mode &= ~(MODE_WRITE << p_off); \
		if (x) \
			p_mode &= ~(MODE_EXEC << p_off); \
	}

	switch (action) {
	case ACTION_SET:
		CHECK_MODE_SET(u, BOF_USER);
		CHECK_MODE_SET(g, BOF_GROUP);
		CHECK_MODE_SET(o, BOF_OTHER);
		break;
	case ACTION_ADD:
		CHECK_MODE_ADD(u, BOF_USER);
		CHECK_MODE_ADD(g, BOF_GROUP);
		CHECK_MODE_ADD(o, BOF_OTHER);
		break;
	case ACTION_SUB:
		CHECK_MODE_SUB(u, BOF_USER);
		CHECK_MODE_SUB(g, BOF_GROUP);
		CHECK_MODE_SUB(o, BOF_OTHER);
	}

	return p_mode;
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

	if (p_argc < 3) {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		try(PROGRAM_NAME);
		return EXIT_FAILURE;
	}
	const char *p_mode_str = p_argv[1];
	int mode = -1;

	if (
		strlen(p_mode_str) < 4 &&
		strspn(p_mode_str, "01234567") == strlen(p_mode_str)
	)
		mode = strtol(p_mode_str, NULL, 8);

	int exitcode  = EXIT_SUCCESS;
	for (p_argv += 2; *p_argv != NULL; ++ p_argv) {
		struct stat entstat;
		if (stat(*p_argv, &entstat) == 0) {
			if (mode == -1) {
				int mode = str_to_mode(p_mode_str, entstat.st_mode);
				if (mode == EXIT_FAILURE_BELOWZERO) {
					error_simple(PROGRAM_NAME);
					error_cleanup();
					try(PROGRAM_NAME);
					return EXIT_FAILURE;
				}
				chmod(*p_argv, mode);
			} else
				chmod(*p_argv, mode);
		} else {
			if (errno == ENOENT) {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			} else {
				ERR_SET_G_ERROR(*p_argv, strerror(errno), ERR_FATAL);
				error_fatal(PROGRAM_NAME);
				return EXIT_FAILURE;
			}
		}
	}

	return exitcode;
}
