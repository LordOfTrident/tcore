#include <stdio.h> /* printf, fopen, fclose, FILE, */
#include <string.h> /* strcmp, strncpy */
#include <stdlib.h> /* EXIT_SUCCESS, EXIT_FAILURE */
#include <unistd.h> /* read */
#include <termios.h> /* termios, tcgetattr, tcsetattr */
#include <errno.h> /* strerror, errno */
#include <stdbool.h> /* bool, true, false */

#include "info.h"
#include "error.h"
#include "utils.h"

/* cursor moving macros */
/* up and down are unused currently */
#define CURUP(p_amount)   printf("\033[%dA", (p_amount))
#define CURDOWN(p_amount) printf("\033[%dB", (p_amount))
#define CURFOR(p_amount)  printf("\033[%dC", (p_amount))
#define CURBACK(p_amount) printf("\033[%dD", (p_amount))

#define CTRL_PLUS(key) (key & 0x1f)

/* keys */
#define KEY_ESCAPE    0x001b
#define KEY_ENTER     0x000a
#define KEY_UP        0x0105
#define KEY_DOWN      0x0106
#define KEY_LEFT      0x0107
#define KEY_RIGHT     0x0108
#define KEY_BACKSPACE 0x007f

#define PROGRAM_NAME "ted"
#define PROGRAM_DESC "Simple file reader and writer."

const char *g_usages[] = {
	"FILE [-r | -w [TEXT]]"
};

t_arg_desc g_arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"--seqs",    "Show the escape sequences"},
	{"-r",        "Read file mode"},
	{"-w",        "Write file mode"},
	{"-w TEXT",   "Set file contents to TEXT (with backslash escaping)"}
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

struct termios g_term;
struct termios g_oterm;

unsigned short getch(void) {
	ERR_WATCH_INIT;
    if (tcgetattr(0, &g_oterm) < 0) {
		ERR_SET_G_ERROR("tcgetattr fail", strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE_BELOWZERO;
	}
    memcpy(&g_term, &g_oterm, sizeof(g_term));

    g_term.c_lflag &= ~(ICANON | ECHO);
    g_term.c_cc[VMIN] = 1;
    g_term.c_cc[VTIME] = 0;

    if (tcsetattr(0, TCSANOW, &g_term) < 0) {
   		ERR_SET_G_ERROR("tcsetattr fail", strerror(errno), ERR_NOT_FATAL);
   		return EXIT_FAILURE_BELOWZERO;
   	}
    char in = getchar();

    if (tcsetattr(0, TCSANOW, &g_oterm) < 0) {
   		ERR_SET_G_ERROR("tcsetattr fail", strerror(errno), ERR_NOT_FATAL);
   		return EXIT_FAILURE_BELOWZERO;
   	}

    return in;
}

unsigned short kbhit(void) {
	ERR_WATCH_INIT;
    if (tcgetattr(0, &g_oterm) < 0) {
		ERR_SET_G_ERROR("tcgetattr fail", strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE_BELOWZERO;
	}
    memcpy(&g_term, &g_oterm, sizeof(g_term));

    g_term.c_lflag &= ~(ICANON | ECHO);
    g_term.c_cc[VMIN] = 0;
    g_term.c_cc[VTIME] = 1;

    if (tcsetattr(0, TCSANOW, &g_term) < 0) {
   		ERR_SET_G_ERROR("tcsetattr fail", strerror(errno), ERR_NOT_FATAL);
   		return EXIT_FAILURE_BELOWZERO;
   	}
    char in = getchar();

    if (tcsetattr(0, TCSANOW, &g_oterm) < 0) {
   		ERR_SET_G_ERROR("tcsetattr fail", strerror(errno), ERR_NOT_FATAL);
   		return EXIT_FAILURE_BELOWZERO;
   	}

	if (in != -1)
		ungetc(in, stdin);
	return (in != -1)? true : false;
}

unsigned short kbesc(void) {
	unsigned short hit = kbhit();
	if (hit == false)
		return KEY_ESCAPE;
	else if (hit == EXIT_FAILURE_BELOWZERO)
		return EXIT_FAILURE_BELOWZERO;

	unsigned short in = getch();
	if (in == EXIT_FAILURE_BELOWZERO)
		return EXIT_FAILURE_BELOWZERO;

	if (in == '[') {
		switch (getch()) {
		case 'A': in = KEY_UP;    break;
		case 'B': in = KEY_DOWN;  break;
		case 'C': in = KEY_LEFT;  break;
		case 'D': in = KEY_RIGHT; break;
		default:  in = 0;
		}
	} else
		in = 0;

	if (in == 0) {
		while ((hit = kbhit()) == true) {
			unsigned short success = getch();
			if (success == EXIT_FAILURE_BELOWZERO)
				return EXIT_FAILURE_BELOWZERO;
		}
		if (hit == EXIT_FAILURE_BELOWZERO)
			return EXIT_FAILURE_BELOWZERO;
	}

	return in;
}

unsigned short kbget(void) {
	unsigned short in = getch();
	if (in == EXIT_FAILURE_BELOWZERO)
		return EXIT_FAILURE_BELOWZERO;
	else
		return (in == KEY_ESCAPE)? kbesc() : in;
}

int disable_ctrlc(void) {
	ERR_WATCH_INIT;
	if (tcgetattr(0, &g_term) < 0) {
		ERR_SET_G_ERROR("tcgetattr fail", strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE;
	}
	g_term.c_lflag &= ~ISIG;

	if (tcsetattr(0, TCSANOW, &g_term) < 0) {
		ERR_SET_G_ERROR("tcgetattr fail", strerror(errno), ERR_NOT_FATAL);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int start(bool p_read, bool p_write, char *p_wcontent, const char *p_path) {
	if (p_read) {
		char *content = read_file(p_path);
		if (content == NULL) {
			if (g_error.fatal)
				error_fatal(PROGRAM_NAME);
			else {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				return EXIT_FAILURE;
			}
		}
		fputs(content, stdout);
	} else if (p_write) {
		bool heap = false;
		bool save = true;
		if (p_wcontent == NULL) {
			puts("Edit mode. Press CTRL+C to exit, ESC to exit and save.");

			size_t chunk_size = CHUNK_SIZE;
			size_t len = 0;
			size_t pos = 0;
			char *line = emalloc(chunk_size + 1);
			*line = '\0';

			size_t wlen = 0;
			p_wcontent = emalloc(1);
			if (line == NULL || p_wcontent == NULL)
				error_fatal(PROGRAM_NAME);
			heap = true;

			unsigned short in;
			bool loop = true;
			while (loop) {
				in = kbget();
				if (in == EXIT_FAILURE_BELOWZERO) {
					free(line);
					free(p_wcontent);
					error_fatal(PROGRAM_NAME);
				}

				switch (in) {
				case CTRL_PLUS('c'):
					loop = false;
					save = false;
					putchar('\n');
					break;
				case KEY_ESCAPE:
					loop = false;
					save = true;
					/* fall through */
				case KEY_ENTER: {
						if (len != 0) {
							void *tmp = erealloc(p_wcontent, wlen + len + 2);
							if (tmp == NULL) {
								free(line);
								free(p_wcontent);
							}
							p_wcontent = tmp;
							strncpy(p_wcontent + wlen, line, len);
							wlen += len;
							if (loop) {
								p_wcontent[wlen] = '\n';
								++ wlen;
								p_wcontent[wlen] = '\0';
							} else
								p_wcontent[wlen] = '\0';

							chunk_size = CHUNK_SIZE;
							len = 0;
							pos = 0;
							tmp = erealloc(line, chunk_size);
							if (tmp == NULL) {
								free(line);
								free(p_wcontent);
								error_fatal(PROGRAM_NAME);
							}
							line = tmp;
						}
						putchar('\n');
						fflush(stdout);
					}
					break;
				case KEY_RIGHT:
					if (pos == 0)
						break;
					-- pos;
					CURBACK(1);

					/* output the rest of the line to move it */
					fputs(line + pos, stdout);
					/* put the cursor back where it was */
					CURBACK((int)strlen(line + pos));
					fflush(stdout);
					break;
				case KEY_LEFT:
					if (pos == len)
						break;
					++ pos;
					CURFOR(1);

					fputs(line + pos, stdout);
					int step = (int)strlen(line + pos);

					/* when the step was 0, CURBACK was problematic */
					if (step != 0)
						CURBACK(step);
					fflush(stdout);
					break;
				case KEY_BACKSPACE:
					-- pos;
					-- len;
					/* move the entire string backward from pos by 1 */
					if (pos != len) {
						for (size_t i = pos; i < len; ++ i)
							line[i] = line[i + 1];
					}
					line[len] = '\0';

					if (pos != len) {
						CURBACK(1);
						fputs(line + pos, stdout);
						putchar(' ');
						CURBACK((int)strlen(line + pos) + 1);
					} else {
						CURBACK(1);
						putchar(' ');
						CURBACK(1);
					}
					fflush(stdout);
					break;
				default:
					if (in > 31 && in < KEY_BACKSPACE) {
						++ len;
						++ pos;
						if (len > chunk_size) {
							chunk_size *= 2;
							void *tmp = erealloc(line, chunk_size + 1);
							if (tmp == NULL) {
								free(line);
								free(p_wcontent);
								error_fatal(PROGRAM_NAME);
							}
							line = tmp;
						}
						/* move the entire forward string from pos by 1 */
						if (pos != len) {
							for (size_t i = len - 1; i >= pos; -- i)
								line[i] = line[i - 1];
						}
						/* insert the char */
						line[pos - 1] = in;
						line[len]     = '\0';

						if (pos != len) {
							fputs(line + pos - 1, stdout);
							CURBACK((int)strlen(line + pos));
						} else
							putchar(in);
						fflush(stdout);
					}
				}
			}
			free(line);
			if (save)
				puts("Exited and saved.");
			else
				puts("Exited.");
		}

		if (save) {
			if (write_file(p_path, p_wcontent, true) != EXIT_SUCCESS) {
				if (g_error.fatal)
					error_fatal(PROGRAM_NAME);
				else {
					error_simple(PROGRAM_NAME);
					error_cleanup();
					return EXIT_FAILURE;
				}
			}
		}
		if (heap)
			free(p_wcontent);
	}

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
	ERR_WATCH_INIT;

	if (disable_ctrlc() != EXIT_SUCCESS)
		error_fatal(PROGRAM_NAME);

	bool read = false;
	bool write = false;
	char *wcontent = NULL;

	if (p_argc < 2) {
		ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
		error_simple(PROGRAM_NAME);
		error_cleanup();
		try(PROGRAM_NAME);
		return EXIT_FAILURE;
	} else if (p_argc > 2) {
		if (strcmp(p_argv[2], "-w") == 0) {
			if (p_argc > 4) {
				ERR_SET_G_ERROR("Extra argument", NULL, ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			}

			if (p_argc == 4)
				wcontent = (char*)p_argv[3];
			write = true;
		} else if (strcmp(p_argv[2], "-r") == 0) {
			if (p_argc > 3) {
				ERR_SET_G_ERROR("Extra argument", NULL, ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			}
			read = true;
		} else {
			ERR_SET_G_ERROR("Extra argument", NULL, ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			return EXIT_FAILURE;
		}
	}

	if (!read && !write) {
		bool loop = true;
		while (loop) {
			printf(
				"To edit '%s' press 'w', to read 'r' and ESC to exit.\n",
				p_argv[1]
			);
			unsigned short in = kbget();
			if (in == EXIT_FAILURE_BELOWZERO)
				error_fatal(PROGRAM_NAME);

			switch (in) {
			case 'w':
				loop = false;
				write = true;
				break;
			case 'r':
				loop = false;
				read = true;
				break;
			case KEY_ESCAPE:
				puts("Exited.");
				return EXIT_SUCCESS;
			default:
				printf("Invalid option '%c'\n", in);
			}
		}
	}

	return start(read, write, wcontent, p_argv[1]);
}
