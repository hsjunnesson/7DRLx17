#include <iomanip>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

#pragma warning(push, 0)
#include "memory.h"
#include "string_stream.h"
#include "temp_allocator.h"
#include <StackWalker.h>
#pragma warning(pop)

using namespace foundation;
using namespace foundation::string_stream;

class LoggingStackWalker : public StackWalker {
  public:
    LoggingStackWalker()
    : StackWalker() {}

  protected:
    virtual void OnOutput(LPCSTR szText) {
        fprintf(stderr, szText);
        StackWalker::OnOutput(szText);
    }
};

void internal_log(LoggingSeverity severity, const char *format, ...) {
    TempAllocator1024 ta;
    Buffer ss(ta);

    const char *severity_prefix = nullptr;
    FILE *stream = nullptr;

    switch (severity) {
    case LoggingSeverity::Debug:
        severity_prefix = " [DEBUG] ";
        stream = stdout;
        break;
    case LoggingSeverity::Info:
        severity_prefix = " [INFO] ";
        stream = stdout;
        break;
    case LoggingSeverity::Error:
        severity_prefix = " [ERROR] ";
        stream = stderr;
        break;
    case LoggingSeverity::Fatal:
        severity_prefix = " [FATAL] ";
        stream = stderr;
        break;
    default:
        return;
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

    if (severity == LoggingSeverity::Fatal || severity == LoggingSeverity::Error) {
        LoggingStackWalker sw;
        sw.ShowCallstack();
    }

    if (severity == LoggingSeverity::Fatal) {
        exit(EXIT_FAILURE);
    }
}
