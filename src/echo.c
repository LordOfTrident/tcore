#include <stdio.h> /* printf, putchar, stdout */
#include <string.h> /* strcmp */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE, strtol */
#include <stdbool.h> /* bool, true, false */
#include <ctype.h> /* isdigit */

#include "info.h"
#include "error.h"

#define PROGRAM_NAME "echo"
#define PROGRAM_DESC "Output the string to standard output."

const char *g_usages[] = {
	"[-n | -e] [STRING...]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"--seqs",    "Show the escape sequences"},
	{"-e",        "Enable backslash escaping"},
	{"-n",        "Do not output a trailling new line"}
};

#define PROGRAM_SEQS_DESC "If any other character is after a backslash, it will\n" \
                          "be ignored and the backslash will be printed."

t_arg_desc g_seq_desc[] = {
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

int main(int p_argc, char **p_argv) {
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
		} else if (strcmp(p_argv[1], "--seqs") == 0) {
			if (
				help(
					NULL,
					NULL, 0,
					PROGRAM_SEQS_DESC,
					g_seq_desc, sizeof(g_seq_desc) / sizeof(t_arg_desc)
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
