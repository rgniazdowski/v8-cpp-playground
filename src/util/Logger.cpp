#include <util/Logger.hpp>
#include <util/Timesys.hpp>

#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace logger
{
    void appendPrefixToBuffer(char *pBuffer, const char *pLevel)
    {
        const time_t ts = timesys::seconds();
        int mstru = (int)timesys::ms() % 1000;
        struct tm ti;
        // struct tm *ti = localtime(&ts);
        localtime_s(&ti, &ts);

        snprintf(pBuffer, BUFFER_MAX - 1, "%02d/%02d/%02d %02d:%02d:%02d.%03d: %7s",
                 ti.tm_mday,
                 ti.tm_mon + 1,
                 ti.tm_year - 100,
                 ti.tm_hour,
                 ti.tm_min,
                 ti.tm_sec,
                 mstru, pLevel);
    } //> appendPrefixToBuffer(...)
    //>-----------------------------------------------------------------------------------
}

unsigned int logger::PrintLog(const char *prefix, const Level level, const char *fmt, ...)
{
    char buffer[BUFFER_MAX];
    va_list args;
#if 0
    if (!log::g_fileOutputLog.isOpen())
        log::g_fileOutputLog.open("outputFull.log", util::FileBase::Mode::WRITE);
    log::g_fileOutputLog.puts(buf);
    log::g_fileOutputLog.puts("\n");
    log::g_fileOutputLog.flush();
#endif
    logger::appendPrefixToBuffer(buffer, getLogLevelName(level).data());
    size_t nLen = strlen(buffer);
    snprintf(buffer + nLen, BUFFER_MAX - nLen - 1, " - %s: ", prefix);
    nLen = strlen(buffer);
    va_start(args, fmt);
    vsnprintf(buffer + nLen, BUFFER_MAX - nLen - 1, fmt, args);
    va_end(args);
    puts(buffer);
    fflush(stdout);
    return (unsigned int)strlen(buffer);
}
//>---------------------------------------------------------------------------------------

unsigned int logger::PrintLog(const Level level, const char *fmt, ...)
{
    char buffer[BUFFER_MAX];
    va_list args;

    logger::appendPrefixToBuffer(buffer, getLogLevelName(level).data());
    size_t nLen = strlen(buffer);
    snprintf(buffer + nLen, BUFFER_MAX - nLen - 1, ": ");
    nLen = strlen(buffer);
    va_start(args, fmt);
    vsnprintf(buffer + nLen, BUFFER_MAX - nLen - 1, fmt, args);
    va_end(args);
    puts(buffer);
    fflush(stdout);
    return (unsigned int)strlen(buffer);
}
//>---------------------------------------------------------------------------------------
