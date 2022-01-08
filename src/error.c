#include "error.h"

/* extern */
t_error g_error = ERR_G_ERROR_INIT;

void error_fatal(const char *p_prog_name) {
	eprintf(
		"%s: fatal error:\n"
		"  why: %s\n"
		"  desc: %s\n"
		"  file: %s\n"
		"  func: %s\n"
		"  line: %lu\n",
		p_prog_name,
		g_error.why,
		g_error.desc,
		g_error.file,
		g_error.func,
		(unsigned long)g_error.line
	);

	if (g_error.heap)
		free(g_error.why);

	eflush();
	exit(EXIT_FAILURE);
}

void error_simple(const char *p_prog_name) {
	if (g_error.desc == NULL)
		eprintf("%s: %s\n", p_prog_name, g_error.why);
	else
		eprintf("%s: %s: %s\n", p_prog_name, g_error.why, g_error.desc);

	eflush();
}

void error_cleanup(void) {
	if (g_error.why != NULL && g_error.heap)
		free(g_error.why);
}

void error_set_g_error(
	char *p_file,
	char *p_func,
	size_t p_line,
	char *p_why,
	char *p_desc,
	bool p_fatal
) {
	error_cleanup();

	g_error.why = malloc(strlen(p_why) + 1);
	if (g_error.why == NULL) {
		g_error.why = (char*)p_why;
		g_error.heap = false;
	} else {
		strcpy(g_error.why, p_why);
		g_error.heap = true;
	}

	g_error.desc  = p_desc;
	g_error.file  = p_file;
	g_error.func  = p_func;
	g_error.line  = p_line;
	g_error.fatal = p_fatal;
}
