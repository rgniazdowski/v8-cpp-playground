#include <util/Logger.hpp>
#include <util/Timesys.hpp>

#include <cstdio>
#include <cstring>
#include <cstdarg>

namespace colors
{
    const char *const black = "\033[30m";   // black
    const char *const red = "\033[31m";     // red
    const char *const green = "\033[32m";   // green
    const char *const yellow = "\033[33m";  // yellow/gold/orange
    const char *const orange = "\033[33m";  // yellow/gold/orange
    const char *const blue = "\033[34m";    // blue
    const char *const magenta = "\033[35m"; // magenta
    const char *const cyan = "\033[36m";    // cyan

    const char *const blackBg = "\033[40m";   // black bg
    const char *const redBg = "\033[41m";     // red bg
    const char *const greenBg = "\033[42m";   // green bg
    const char *const yellowBg = "\033[43m";  // yellow bg
    const char *const orangeBg = "\033[43m";  // yellow bg
    const char *const blueBg = "\033[44m";    // blue bg
    const char *const magentaBg = "\033[45m"; // magenta bg
    const char *const cyanBg = "\033[46m";    // cyan bg

    const char *const reset = "\033[0m";
    const char *const zero = "\033[0m";
    const char *const blank = "\033[0m";
} //> namespace colors

namespace logger
{
    const char *getColorForLevel(const Level level)
    {
        if (level == Level::Trace)
            return colors::magenta;
        else if (level == Level::All)
            return colors::blue;
        else if (level == Level::Debug)
            return colors::cyan;
        else if (level == Level::Info)
            return colors::green;
        else if (level == Level::Warning)
            return colors::orange;
        else if (level == Level::Error)
            return colors::red;
        else if (level == Level::Fatal)
            return colors::blue;
        return colors::reset;
    } //> getColorForLevel(...)
    //>-----------------------------------------------------------------------------------

    void appendPrefixToBuffer(char *pBuffer, const char *pLevel, const Level level, bool useColor)
    {
        const time_t ts = timesys::seconds();
        int mstru = ((uint64_t)timesys::ms()) % 1000;
        struct tm ti;
        // struct tm *ti = localtime(&ts);
        localtime_s(&ti, &ts);

        snprintf(pBuffer, BUFFER_MAX - 1, "%s%02d-%02d-%02d %02d:%02d:%02d.%03d %s[%s%7s%s] ",
                 /**/ useColor ? colors::orange : "",
                 ti.tm_mday,
                 ti.tm_mon + 1,
                 ti.tm_year - 100,
                 ti.tm_hour,
                 ti.tm_min,
                 ti.tm_sec,
                 mstru,
                 /**/ useColor ? colors::zero : "",
                 /**/ useColor ? getColorForLevel(level) : "",
                 pLevel,
                 /**/ useColor ? colors::zero : "");
    } //> appendPrefixToBuffer(...)
    //>-----------------------------------------------------------------------------------
} //> namespace logger

unsigned int logger::PrintLog(const char *prefix, const Level level, const char *fmt, ...)
{
    char buffer[BUFFER_MAX];
    va_list args;
    bool useColor = true;
#if 0
    if (!log::g_fileOutputLog.isOpen())
        log::g_fileOutputLog.open("outputFull.log", util::FileBase::Mode::WRITE);
    log::g_fileOutputLog.puts(buf);
    log::g_fileOutputLog.puts("\n");
    log::g_fileOutputLog.flush();
#endif
    logger::appendPrefixToBuffer(buffer, getLogLevelName(level).data(), level, useColor /* FIXME */);
    size_t nLen = strlen(buffer);
    snprintf(buffer + nLen, BUFFER_MAX - nLen - 1, "%s[%s%s%s] ",
             useColor ? colors::zero : "",
             useColor ? colors::magenta : "",
             prefix,
             useColor ? colors::zero : "");
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
    bool useColor = true;
    logger::appendPrefixToBuffer(buffer, getLogLevelName(level).data(), level, useColor /* FIXME */);
    size_t nLen = strlen(buffer);
    va_start(args, fmt);
    vsnprintf(buffer + nLen, BUFFER_MAX - nLen - 1, fmt, args);
    va_end(args);
    puts(buffer);
    fflush(stdout);
    return (unsigned int)strlen(buffer);
}
//>---------------------------------------------------------------------------------------
