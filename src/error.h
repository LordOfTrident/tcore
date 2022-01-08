#ifndef ERROR_H__HEADER_GUARD__
#define ERROR_H__HEADER_GUARD__

#include <stdio.h> /* fprintf, fflush, size_t */
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdbool.h> /* bool, true, false */
#include <string.h> /* strlen, strcpy */

/*
 *  g_error keeps track of where the "why" string is located - on the heap or on
 *  the stack. error_set_g_error will free the string if it is on the heap and
 *  allocate a new one on the heap if possible. error_fatal will free the string
 *  while error_simple will NOT, run error_cleanup before exiting.
 *
 */

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define eflush()     fflush(stderr)

#define EXIT_FAILURE_BELOWZERO -1

/* macros for more consistency */
#define ERR_FILE __FILE__
#define ERR_FUNC (char*)__func__
#define ERR_LINE __LINE__

/* error tools */
#define ERR_WATCH __err_line__ = ERR_LINE
#define ERR_WATCH_LINE __err_line__
#define ERR_WATCH_INIT size_t __err_line__ = 0

#define ERR_FATAL     true
#define ERR_NOT_FATAL false

#define ERR_SET_G_ERROR(p_why, p_desc, p_fatal) \
	error_set_g_error( \
		ERR_FILE, \
		ERR_FUNC, \
		ERR_WATCH_LINE, \
		(char*)p_why, \
		(char*)p_desc, \
		p_fatal \
	)

#define ERR_G_ERROR_INIT { \
		NULL,  /* why */ \
		NULL,  /* desc */ \
		NULL,  /* func */ \
		NULL,  /* file */ \
		0,     /* line */ \
		false, /* fatal */ \
		false, /* heap */ \
	}

typedef struct {
	char *why;
	char *desc;
	char *func;
	char *file;
	size_t line;
	bool fatal;
	bool heap;
} t_error;
extern t_error g_error;

void error_fatal(const char *p_prog_name);
void error_simple(const char *p_prog_name);
void error_cleanup(void);
void error_set_g_error(
	char *p_file,
	char *p_func,
	size_t p_line,
	char *p_why,
	char *p_desc,
	bool p_fatal
);
#endif /* ERROR_H__HEADER_GUARD__ */
