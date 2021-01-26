#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void internal_log(LOG_SEVERITY severity, const char *format, ...) {
	FILE *stream = nullptr;
	switch (severity) {
	case DEBUG:
		stream = stdout;
		break;
	case INFO:
		stream = stdout;
		break;
	case ERROR:
		stream = stderr;
		break;
	}

	va_list(args);
	va_start(args, format);
	vfprintf(stream, format, args);
	va_end(args);

	fprintf(stderr, "\n");
}
