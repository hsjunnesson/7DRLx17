#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <iomanip>

#include "log.h"
#include "memory.h"
#include "string_stream.h"
#include "temp_allocator.h"

using namespace foundation;
using namespace foundation::string_stream;

void internal_log(LOG_SEVERITY severity, const char* format, ...) {
	TempAllocator1024 ta;
	Buffer ss(ta);

	const char* severity_prefix = nullptr;
	FILE* stream = nullptr;

	switch (severity) {
	case LOG_SEVERITY::DEBUG:
		severity_prefix = " [DEBUG] ";
		stream = stdout;
		break;
	case LOG_SEVERITY::INFO:
		severity_prefix = " [INFO] ";
		stream = stdout;
		break;
	case LOG_SEVERITY::ERROR:
		severity_prefix = " [ERROR] ";
		stream = stderr;
		break;
	case LOG_SEVERITY::FATAL:
		severity_prefix = " [FATAL] ";
		stream = stderr;
		break;
	}

	{
		struct tm time_info;
		__time64_t long_time;
		_time64(&long_time);
		char timebuf[100];
		if (!_localtime64_s(&time_info, &long_time)) {
			std::strftime(timebuf, sizeof(timebuf), "%FT%T", &time_info);
			ss << timebuf;
		}
	}

	ss << severity_prefix;

	va_list(args);
	va_start(args, format);
	ss = vprintf(ss, format, args);
	va_end(args);

	ss << "\n";

	fprintf(stream, c_str(ss));
	fflush(stream);
}
