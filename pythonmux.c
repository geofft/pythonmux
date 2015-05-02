#define _GNU_SOURCE
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *optstring = "+bBc:dEhiIJm:OqRsStuvVW:xX:?";
static const struct option long_options[] = {
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{NULL, 0, NULL, 0},
};

static const char *marker = "pyversions";
static int marker_len = 10;

struct version {
	int major;
	int minor;
};

static const struct version versions[] = {
	{3, 4}, {3, 3}, {3, 2}, {3, 1}, {3, 0},
	{2, 7}, {2, 6},
};

static char *
find_pyversions_str(char *buf)
{
	while (buf && *buf) {
		buf = strstr(buf, marker);
		if (!buf) {
			return NULL;
		}
		buf = buf + marker_len;
		if (*buf != ':' &&
		    *buf != '=') {
			continue;
		}
		buf = buf + 1 + strspn(buf, " \t\n\r\f\v");
		char *end = buf + strspn(buf, "0123456789.+,");
		if (buf == end) {
			continue;
		}
		*end = '\0';
		return buf;
	}
	return NULL;
}

static char *
find_pyversions(FILE *input)
{
	char buf[4096];
	char *pyversions = NULL;
	int i;
	for (i = 0; i < 2; i++) {
		if (!fgets(buf, 4096, input)) {
			break;
		}
		if ((pyversions = find_pyversions_str(buf))) {
			break;
		}
	}
	if (pyversions) {
		return strdup(pyversions);
	} else {
		return NULL;
	}
}

static int
matches(int major, int minor, char *pyversions)
{
	while (pyversions && *pyversions) {
		int target_major, target_minor, chars;
		int inexact = 0;
		int r = sscanf(pyversions, "%d.%d%n",
			       &target_major, &target_minor, &chars);
		pyversions += chars;
		if (*pyversions == '+') {
			inexact = 1;
			pyversions++;
		}
		if (*pyversions == ',') {
			pyversions++;
		}
		if (r < 2) {
			continue;
		}
		if (major != target_major) {
			continue;
		}
		if (minor < target_minor) {
			continue;
		}
		if (!inexact && (minor != target_minor)) {
			continue;
		}
		return 1;
	}
	return 0;
}

static void __attribute__((noreturn))
exec_python(char *pyversions, char *argv[])
{
	unsigned i;
	char *python = NULL;
	if (!pyversions) {
		pyversions = strdup("2.0+");
	}
	for (i = 0; i < sizeof(versions) / sizeof(*versions); i++) {
		asprintf(&python, "/usr/bin/python%d.%d",
		         versions[i].major, versions[i].minor);
		if (access(python, X_OK) == 0 &&
		    matches(versions[i].major, versions[i].minor, pyversions)) {
			break;
		} else {
			free(python);
			python = NULL;
		}
	}
	if (python) {
		execvp(python, argv);
		fprintf(stderr, "python: exec %s: %s\n", python, strerror(errno));
		free(python);
	} else {
		fprintf(stderr, "python: No installed version of Python matches `%s'\n", pyversions);
	}
	free(pyversions);
	exit(1);
}

int
main(int argc, char *argv[])
{
	int c;
	int ignore_environment = 0;
	int scripted_mode = 0;
	int interactive = isatty(STDIN_FILENO);
	errno = 0;
	while ((c = getopt_long(argc, argv, optstring, long_options, NULL)) != -1) {
		if (c == 'c' || c == 'm') {
			/* further arguments are for the script/module */
			scripted_mode = 1;
		}
		if (c == 'E') {
			ignore_environment = 1;
			break;
		}
		// TODO check for errors
		// TODO check for BSD/OS X compat
	}

	if (!scripted_mode &&
	    optind < argc &&
	    strcmp(argv[optind], "-") != 0) {
		char *pyversions = NULL;
		FILE *input = fopen(argv[optind], "r");
		if (input) {
			pyversions = find_pyversions(input);
			fclose(input);
		}
		/* If we can't open the file, just go with the default
		 * (Python 2) */
		exec_python(pyversions, argv);
	}

	if (!interactive &&
	    lseek(STDIN_FILENO, 0, SEEK_CUR) == -1) {
		errno = 0;
		/* Since we could not seek, we can't safely read the
		 * first two lines and still pass the whole thing onto
		 * the real Python interpreter. That means either we're
		 * interactive (which we checked for with isatty), or
		 * something like `echo 'print "hi"' | python`, which we
		 * should treat like `python -c 'print "hi"'`. */
		scripted_mode = 1;
	}

	if (scripted_mode) {
		if (ignore_environment) {
			exec_python(NULL, argv);
		} else {
			char *pyversions = getenv("PYVERSIONS");
			if (pyversions) {
				exec_python(strdup(pyversions), argv);
			} else {
				exec_python(NULL, argv);
			}
		}
	}

	if (interactive) {
		exec_python(strdup("2.0+,3.0+"), argv);
	};

	exec_python(find_pyversions(stdin), argv);
}
