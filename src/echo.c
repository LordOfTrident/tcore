#include <stdio.h> /* printf, fputs, fputc, putchar, stdout */
#include <string.h> /* strcmp, strchr */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, strtol */
#include <stdbool.h> /* bool, true, false */
#include <ctype.h> /* isdigit */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "echo"
#define PROGRAM_DESC "Output the string to standard output."

const char *usages[] = {
	"[-n | -e] [STRING...]"
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"--seqs",    "Show the escape sequences"},
	{"-e",        "Enable backslash escaping"},
	{"-n",        "Do not output a trailling new line"}
};

#define PROGRAM_SEQS_DESC "If any other character is after a backslash, it will\n" \
                          "be ignored and the backslash will be printed."

t_arg_desc seq_desc[] = {
	{"\\\\",   "Backslash"},
	{"\\a",    "Alert"},
	{"\\b",    "Backspace"},
	{"\\c",    "Cut output here"},
	{"\\e",    "Escape"},
	{"\\f",    "Form feed"},
	{"\\n",    "New line"},
	{"\\r",    "Carriage return"},
	{"\\t",    "Horizontal tab"},
	{"\\v",    "Vertical tab"},
	{"\\0NNN", "Octal number (1 to 3 digits)"}
};

bool fputse(char *p_str, FILE *p_stream) {
	char *tmp = p_str;
	while ((tmp = strchr(tmp, '\\')) != NULL) {
		*tmp = '\0';
		fputs(p_str, p_stream);
		switch (tmp[1]) {
		case 'a':  fputc('\a', stdout); break;
		case 'b':  fputc('\b', stdout); break;
		case 'f':  fputc('\f', stdout); break;
		case 'n':  fputc('\n', stdout); break;
		case 'r':  fputc('\r', stdout); break;
		case 't':  fputc('\t', stdout); break;
		case 'v':  fputc('\v', stdout); break;
		case '\\': fputc('\\', stdout); break;
		case 'c': return false; break;
		case '0': {
				char num[5] = {0};
				tmp += 2;
				for (int i = 0; *tmp >= '0' && *tmp <= '7' && i < 3; ++ i, ++ tmp)
					num[i] = *tmp;
				fputc(strtol(num, NULL, 8), stdout);

				p_str = tmp;
				continue;
			}
			break;
		default:
			putchar('\\');

			++ tmp;
			p_str = tmp;
			continue;
		}

		tmp += 2;
		p_str = tmp;
	}
	fputs(p_str, p_stream);

	return true;
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
		} else if (strcmp(p_argv[1], "--seqs") == 0) {
			if (
				help(
					NULL,
					NULL, 0,
					PROGRAM_SEQS_DESC,
					seq_desc, sizeof(seq_desc) / sizeof(t_arg_desc)
				) != EXIT_SUCCESS
			)
				error_fatal(PROGRAM_NAME);
			else
				return EXIT_SUCCESS;
		}
	}

	bool newline = true;
	bool options = true;
	bool escape  = false;
	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		if (options) {
			if (strcmp(*p_argv, "-n") == 0) {
				newline = false;
				continue;
			} else if (strcmp(*p_argv, "-e") == 0) {
				escape = true;
				continue;
			}
			options = false;
		}

		if (escape) {
			if (!fputse(*p_argv, stdout))
				newline = false;
		} else
			fputs(*p_argv, stdout);
		/* Dont output the last useless space, it
		   might bug the output                   */
		if (*(p_argv + 1) != NULL)
			putchar(' ');
	}
	if (newline)
		putchar('\n');

	return EXIT_SUCCESS;
}
