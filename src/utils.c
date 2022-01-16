#include "utils.h"

void *emalloc(size_t p_bytes) {
	ERR_WATCH_INIT;
	void *buf = malloc(p_bytes);
	if (buf == NULL) {
		ERR_SET_G_ERROR("malloc fail", strerror(errno), ERR_FATAL);
		return NULL;
	}

	return buf;
}

void *erealloc(void *p_buf, size_t p_bytes) {
	ERR_WATCH_INIT;
	void *tmp = realloc(p_buf, p_bytes);
	if (tmp == NULL) {
		ERR_SET_G_ERROR("realloc fail", strerror(errno), ERR_FATAL);
		return NULL;
	}

	return tmp;
}

bool fputse(const char *p_str, FILE *p_stream) {
	char *orig = strclone(p_str);
	char *out  = orig;
	char *tmp  = orig;
	while ((tmp = strchr(tmp, '\\')) != NULL) {
		*tmp = '\0';
		fputs(out, p_stream);
		switch (tmp[1]) {
		case 'a':  fputc('\a', p_stream); break;
		case 'b':  fputc('\b', p_stream); break;
		case 'f':  fputc('\f', p_stream); break;
		case 'n':  fputc('\n', p_stream); break;
		case 'r':  fputc('\r', p_stream); break;
		case 't':  fputc('\t', p_stream); break;
		case 'v':  fputc('\v', p_stream); break;
		case '\\': fputc('\\', p_stream); break;
		case 'c': return false; break;
		case '0': {
				char num[5] = {0};
				tmp += 2;
				for (int i = 0; *tmp >= '0' && *tmp <= '7' && i < 3; ++ i, ++ tmp)
					num[i] = *tmp;
				fputc(strtol(num, NULL, 8), p_stream);

				out = tmp;
				continue;
			}
			break;
		default:
			putchar('\\');

			++ tmp;
			out = tmp;
			continue;
		}

		tmp += 2;
		out = tmp;
	}
	fputs(out, p_stream);

	free(orig);
	return true;
}

char *read_file(const char *p_path) {
	ERR_WATCH_INIT;
	struct stat entstat;
	ERR_WATCH; if (stat(p_path, &entstat) == 0) {
		if (S_ISDIR(entstat.st_mode) != 0) {
			ERR_SET_G_ERROR(p_path, "Is a directory", ERR_NOT_FATAL);
			return NULL;
		}
	} else {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_NOT_FATAL);
		return NULL;
	}

	ERR_WATCH; FILE *fhnd = fopen(p_path, "r");
	if (fhnd == NULL) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_FATAL);
		return NULL;
	}

	fseek(fhnd, 0, SEEK_END);
	size_t fsize = ftell(fhnd);
	fseek(fhnd, 0, SEEK_SET);

	char* fcontent = (char*)emalloc(fsize + 1);
	if (fcontent == NULL)
		return NULL;
	for (char *p = fcontent; (*p = getc(fhnd)) != EOF; ++ p);
	fcontent[fsize] = '\0';

	fclose(fhnd);
	return fcontent;
}

int write_file(const char *p_path, const char *p_content, bool p_escaped) {
	ERR_WATCH_INIT;
	struct stat entstat;
	ERR_WATCH; if (stat(p_path, &entstat) == 0) {
		if (S_ISDIR(entstat.st_mode) != 0) {
			ERR_SET_G_ERROR(p_path, "Is a directory", ERR_NOT_FATAL);
			return EXIT_FAILURE;
		}
	} else if (errno != ENOENT) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_FATAL);
		return EXIT_FAILURE;
	}

	ERR_WATCH; FILE *fhnd = fopen(p_path, "w");
	if (fhnd == NULL) {
		ERR_SET_G_ERROR(p_path, strerror(errno), ERR_FATAL);
		return EXIT_FAILURE;
	}

	if (p_escaped)
		fputse(p_content, fhnd);
	else
		fputs(p_content, fhnd);
	fclose(fhnd);
	return EXIT_SUCCESS;
}

/* not apart of c99 standard */
char *strclone(const char *p_str) {
	char *buf = (char*)emalloc(strlen(p_str) + 1);
	if (buf == NULL)
		return NULL;

	strcpy(buf, p_str);
	return buf;
}
