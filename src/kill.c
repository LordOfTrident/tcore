/* macro for kill */
#define _POSIX_SOURCE

#include <stdio.h> /* printf, fprintf, putchar */
#include <string.h> /* strcmp, strspn, snprintf */
#include <stdlib.h> /* strtol, EXIT_SUCCESS, EXIT_FAILURE */
#include <sys/types.h> /* pid_t */
#include <signal.h> /* kill, strsignal, SIGKILL */
#include <ctype.h> /* toupper */

#include "info.h"
#include "error.h"
#include "utils.h"

#define PROGRAM_NAME "kill"
#define PROGRAM_DESC "Send a signal to a process. Default signal is SIGTERM."

const char *usages[] = {
	"[-s SIGNAL | -n SIGNUM] PID...",
	"-l [SIGNAL... | SIGNUM...]"
};

t_arg_desc arg_desc[] = {
	{"--help",    "Show command help"},
	{"--version", "Show tcore version"},
	{"-l",        "List all the signal names and their numbers;\n"
	              "if arguments follow they are either taken as signal\n"
	              "numbers or names and translated to the opposite"},
	{"-s SIGNAL", "set signal to SIGNAL (signal name)"},
	{"-n SIGNUM", "set signal to SIGNUM (signal number)"}
};

char *signals[65] = {
    "", /* 0 */
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGABRT",
	"SIGBUS",
	"SIGFPE",
	"SIGKILL",
	"SIGUSR1",
	"SIGSEGV",
	"SIGUSR2",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGSTKFLT",
	"SIGCHLD",
	"SIGCONT",
	"SIGSTOP",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGURG",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGIO",
	"SIGPWR",
	"", /* 32 */
	"", /* 33 */
	"SIGSYS",
	"SIGRTMIN",
	"SIGRTMIN+1",
	"SIGRTMIN+2",
	"SIGRTMIN+3",
	"SIGRTMIN+4",
	"SIGRTMIN+5",
	"SIGRTMIN+6",
	"SIGRTMIN+7",
	"SIGRTMIN+8",
	"SIGRTMIN+9",
	"SIGRTMIN+10",
	"SIGRTMIN+11",
	"SIGRTMIN+12",
	"SIGRTMIN+13",
	"SIGRTMIN+14",
	"SIGRTMIN+15",
	"SIGRTMAX-14",
	"SIGRTMAX-13",
	"SIGRTMAX-12",
	"SIGRTMAX-11",
	"SIGRTMAX-10",
	"SIGRTMAX-9",
	"SIGRTMAX-8",
	"SIGRTMAX-7",
	"SIGRTMAX-6",
	"SIGRTMAX-5",
	"SIGRTMAX-4",
	"SIGRTMAX-3",
	"SIGRTMAX-2",
	"SIGRTMAX-1",
	"SIGRTMAX"
};

int sig_str_to_num(const char *p_str) {
	ERR_WATCH_INIT;
	int signal = EXIT_FAILURE_BELOWZERO;
	char *str = strclone(p_str);
	if (str == NULL)
		return EXIT_FAILURE_BELOWZERO;

	for (char *p = str; *p != '\0'; ++ p)
		*p = toupper(*p);

	ERR_WATCH; for (int i = 1; i < sizeof(signals) / sizeof(char*); ++ i) {
		if (signals[i][0] != '\0') {
			if (strcmp(str, signals[i]) == 0) {
				signal = i;
				break;
			}
		}
	}
	if (signal == EXIT_FAILURE_BELOWZERO) {
		ERR_SET_G_ERROR(str, "Invalid signal", ERR_NOT_FATAL);
	}

	/* if not found, signal is EXIT_FAILURE_BELOWZERO anyways, so we can just
	   return it                                                              */
	free(str);
	return signal;
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

	int signal = SIGTERM;
	int exitcode = EXIT_SUCCESS;
	if (strcmp(p_argv[1], "-l") == 0) {
		if (p_argc > 2) {
			for (p_argv += 2; *p_argv != NULL; ++ p_argv) {
				if (strspn(*p_argv, "0123456789") == strlen(*p_argv)) {
					long signum = strtol(*p_argv, NULL, 10);
					if (signum < 65)
						printf("%s\n", signals[signum]);
					else {
						ERR_SET_G_ERROR(*p_argv, "Invalid signal", ERR_NOT_FATAL);
						error_simple(PROGRAM_NAME);
						error_cleanup();
						try(PROGRAM_NAME);
						exitcode = EXIT_FAILURE;
					}
				} else {
					signal = sig_str_to_num(*p_argv);
					if (signal == EXIT_FAILURE_BELOWZERO) {
						error_simple(PROGRAM_NAME);
						error_cleanup();
						try(PROGRAM_NAME);
						exitcode = EXIT_FAILURE;
					} else
						printf("%i\n", signal);
				}
			}
		} else {
			for (int i = 1; i < sizeof(signals) / sizeof(char*); ++ i) {
				if (signals[i][0] != '\0')
					printf("%i) %s   ", i, signals[i]);
			}
			putchar('\n');
		}
		return exitcode;
	} else if (strcmp(p_argv[1], "-n") == 0) {
		if (p_argc > 2) {
			if (strspn(p_argv[2], "0123456789") != strlen(p_argv[2])) {
				ERR_SET_G_ERROR(p_argv[2], "Invalid signal", ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			}

			long signum = strtol(p_argv[2], NULL, 10);
			if (signum < 65)
				signal = signum;
			else {
				ERR_SET_G_ERROR(p_argv[2], "Invalid signal", ERR_NOT_FATAL);
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			}

			p_argv += 2;
			p_argc -= 2;
		} else {
			ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			return EXIT_FAILURE;
		}
	} else if (strcmp(p_argv[1], "-s") == 0) {
		if (p_argc > 2) {
			signal = sig_str_to_num(p_argv[2]);
			if (signal == EXIT_FAILURE_BELOWZERO) {
				error_simple(PROGRAM_NAME);
				error_cleanup();
				try(PROGRAM_NAME);
				return EXIT_FAILURE;
			}

			p_argv += 2;
			p_argc -= 2;
		} else {
			ERR_SET_G_ERROR("Missing argument", NULL, ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			return EXIT_FAILURE;
		}
	}

	for (++ p_argv; *p_argv != NULL; ++ p_argv) {
		ERR_WATCH; if (strspn(*p_argv, "0123456789") != strlen(*p_argv)) {
			ERR_SET_G_ERROR(*p_argv, "Expected a pid", ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			exitcode = EXIT_FAILURE;
			continue;
		}

		pid_t pid = strtol(*p_argv, NULL, 10);
		ERR_WATCH; if (kill(pid, signal) != 0) {
			char str_pid[32];
			snprintf(str_pid, sizeof(str_pid), "%li", (long)pid);
			ERR_SET_G_ERROR(str_pid, "Unknown pid", ERR_NOT_FATAL);
			error_simple(PROGRAM_NAME);
			error_cleanup();
			try(PROGRAM_NAME);
			exitcode = EXIT_FAILURE;
		}
	}

	return exitcode;
}
