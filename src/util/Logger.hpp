#pragma once
#ifndef FG_INC_LOGGER
#define FG_INC_LOGGER

#include <BuildConfig.hpp>
#include <util/Tag.hpp>
#include <magic_enum.hpp>

namespace logger
{
    enum class Level : unsigned int
    {
        Trace = 0,
        All = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Fatal = 5,
        Off = 6
    };
    inline extern Level level = Level::Info;
    const unsigned int BUFFER_MAX = 4096;

    unsigned int PrintLog(const char *prefix, const Level level, const char *fmt, ...);
    unsigned int PrintLog(const Level level, const char *fmt, ...);

    template <typename... Args>
    static unsigned int PrintDebug(const char *fmt, Args... args)
    {
        return PrintLog(Level::Debug, fmt, args...);
    }
    template <typename... Args>
    static unsigned int PrintInfo(const char *fmt, Args... args)
    {
        return PrintLog(Level::Info, fmt, args...);
    }
    template <typename... Args>
    static unsigned int PrintWarning(const char *fmt, Args... args)
    {
        return PrintLog(Level::Warning, fmt, args...);
    }
    template <typename... Args>
    static unsigned int PrintTrace(const char *fmt, Args... args)
    {
        return PrintLog(Level::Trace, fmt, args...);
    }
    template <typename... Args>
    static unsigned int PrintError(const char *fmt, Args... args)
    {
        return PrintLog(Level::Error, fmt, args...);
    }

    template <class TagType, Level DefaultLevel = Level::Info>
    struct Logger
    {
        static_assert(std::is_base_of<util::TagBase, TagType>::value, "TagType template parameter type needs to be derived from TagBase");
        using tag_type = TagType;
        using user_type = typename tag_type::user_type;

        static inline Level level = DefaultLevel;

    private:
        Logger() = default;
        ~Logger() {}

    public:
        template <typename... Args>
        static unsigned int debug(const char *fmt, Args... args)
        {
            return (level <= Level::Debug ? PrintLog(tag_type::name(), Level::Debug, fmt, args...) : 0);
        }
        template <typename... Args>
        static unsigned int info(const char *fmt, Args... args)
        {
            return (level <= Level::Info ? PrintLog(tag_type::name(), Level::Info, fmt, args...) : 0);
        }
        template <typename... Args>
        static unsigned int warning(const char *fmt, Args... args)
        {
            return (level <= Level::Warning ? PrintLog(tag_type::name(), Level::Warning, fmt, args...) : 0);
        }
        template <typename... Args>
        static unsigned int trace(const char *fmt, Args... args)
        {
            return (level <= Level::Trace ? PrintLog(tag_type::name(), Level::Trace, fmt, args...) : 0);
        }
        template <typename... Args>
        static unsigned int error(const char *fmt, Args... args)
        {
            return (level <= Level::Error ? PrintLog(tag_type::name(), Level::Error, fmt, args...) : 0);
        }
        template <typename... Args>
        static unsigned int fatal(const char *fmt, Args... args)
        {
            return (level <= Level::Fatal ? PrintLog(tag_type::name(), Level::Fatal, fmt, args...) : 0);
        }
    }; //> struct Logger

    inline extern const char *DEFAULT_FOLDER = "log\0";

    inline bool isLogLevelTestValid(std::string_view text)
    {
        return magic_enum::enum_cast<Level>(text).has_value();
    }

    inline Level getLogLevelFromText(std::string_view text)
    {
        auto level = magic_enum::enum_cast<Level>(text);
        return (level.has_value() ? level.value() : Level::Info);
    }

    inline std::string_view getLogLevelName(Level level)
    {
        return magic_enum::enum_name(level);
    }

} //> namespace logger

#endif //> FG_INC_LOGGER