#include <stdio.h> /* printf, fputs, putchar */
#include <string.h> /* strcmp, memset */
#include <sys/ioctl.h> /* ioctl, TIOCGWINSZ */
#include <unistd.h> /* STDOUT_FILENO */
#include <stdlib.h> /* free, EXIT_SUCCESS, EXIT_FAILURE */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "align"
#define PROGRAM_DESC "Align words from piped input split by spaces or new lines.\n"\
                     "The ' characters marks the start/end of a string. In a\n"\
                     "string, all characters, except new line characters which\n"\
                     "will cut the string off, are taken as a single word."

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

	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	size_t term_width = w.ws_col - 1;

	size_t str_len = 0;
	size_t str_chunk_size = CHUNK_SIZE;
	char *str = emalloc(str_chunk_size);

	size_t buf_len = 0;
	char **buf = emalloc(1);

	if (str == NULL || buf == NULL)
		error_fatal(PROGRAM_NAME);

	bool in_str = false;
	char ch = getchar();
	size_t longest_len = 0;
	while (ch != EOF) {
		if (ch == '\n' || (ch == ' ' && !in_str)) {
			while (ch == '\n' || (ch == ' ' && !in_str))
				ch = getchar();

			++ buf_len;
			void *tmp = erealloc(buf, buf_len * sizeof(char*));
			if (tmp == NULL) {
				free(buf);
				error_fatal(PROGRAM_NAME);
			}
			buf = tmp;

			++ str_len;
			if (str_len > str_chunk_size) {
				void *tmp = erealloc(str, str_chunk_size + 1);
				if (tmp == NULL) {
					free(str);
					error_fatal(PROGRAM_NAME);
				}
				str = tmp;
			}
			str[str_len - 1] = 0;

			size_t len = strlen(str);
			if (len > longest_len)
				longest_len = len;

			tmp = strclone(str);
			if (tmp == NULL)
				error_fatal(PROGRAM_NAME);
			buf[buf_len - 1] = tmp;

			str_len = 0;
			str_chunk_size = CHUNK_SIZE;
		} else {
			if (ch == '\'')
				in_str = !in_str;

			++ str_len;
			if (str_len > str_chunk_size) {
				str_chunk_size *= 2;
				void *tmp = erealloc(str, str_chunk_size);
				if (tmp == NULL) {
					free(str);
					error_fatal(PROGRAM_NAME);
				}
				str = tmp;
			}
			str[str_len - 1] = ch;

			ch = getchar();
		}
	}
	if (str_len != 0) {
		++ str_len;
		if (str_len > str_chunk_size) {
			void *tmp = erealloc(str, str_chunk_size + 1);
			if (tmp == NULL) {
				free(str);
				error_fatal(PROGRAM_NAME);
			}
			str = tmp;
		}
		str[str_len - 1] = 0;

		++ buf_len;
		void *tmp = erealloc(buf, buf_len * sizeof(char*));
		if (tmp == NULL) {
			free(buf);
			error_fatal(PROGRAM_NAME);
		}
		buf = tmp;

		buf[buf_len - 1] = str;
	} else
		free(str);

	size_t len = 0;
	size_t space_len = 0;
	char *space = emalloc(1);
	if (space == NULL) {
		free(buf);
		error_fatal(PROGRAM_NAME);
	}

	for (size_t i = 0; i < buf_len; ++ i) {
		size_t str_len = strlen(buf[i]);
		len += str_len;
		if (len > term_width && len > longest_len) {
			len = str_len;
			putchar('\n');
		}

		space_len = longest_len - str_len + 1;
		void *tmp = erealloc(space, space_len + 1);
		if (tmp == NULL) {
			free(buf);
			free(space);
			error_fatal(PROGRAM_NAME);
		}
		space = tmp;
		memset(space, ' ', space_len);
		space[space_len] = '\0';

		fputs(buf[i], stdout);
		len += space_len;
		if (len > term_width && len > longest_len) {
			len = 0;
			if (i + 1 != buf_len)
				putchar('\n');
		} else
			fputs(space, stdout);

		free(buf[i]);
	}
	free(buf);
	free(space);
	putchar('\n');

	return EXIT_SUCCESS;
}
