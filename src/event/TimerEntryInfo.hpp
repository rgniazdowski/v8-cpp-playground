#pragma once
#ifndef FG_INC_TIMER_ENTRY_INFO
#define FG_INC_TIMER_ENTRY_INFO

#include <event/EventHelper.hpp>
#include <util/timesys.hpp>

namespace event
{
    struct TimerHelper;

    using WrappedArgs = std::vector<::util::WrappedValue *>;
    using WrappedArgsPtrVec = std::vector<WrappedArgs *>;

    struct TimerEntryInfo
    {
        friend struct TimerHelper;

        inline static uint32_t s_autoid = 0;

        inline static const int INFINITE = -1;

        enum Type
        {
            INTERVAL,
            TIMEOUT
        };

    protected:
        Type type;
        uint32_t id;

    public:
        int timeout;
        int repeats;
        int64_t startTs;
        bool triggered;
        WrappedArgs args;

    protected:
        std::unique_ptr<util::Callback> callback;

    public:
        TimerEntryInfo() : type(INTERVAL), id(autoid()), timeout(0), repeats(0), startTs(timesys::ticks()), triggered(false), args(), callback() {}
        TimerEntryInfo(uint32_t _id, int _repeats, int _timeout, util::Callback *_pCallback)
            : type(INTERVAL), id(!_id ? TimerEntryInfo::autoid() : _id), timeout(_timeout), repeats(_repeats), startTs(timesys::ticks()), triggered(false), args(), callback(_pCallback) {}

        ~TimerEntryInfo()
        {
            util::reset_arguments(args);
            id = 0;
            timeout = 0;
            repeats = 0;
            startTs = 0;
            triggered = false;
        }

        TimerEntryInfo(const TimerEntryInfo &other) = delete;
        TimerEntryInfo &operator=(const TimerEntryInfo &other) = delete;

        TimerEntryInfo(TimerEntryInfo &&other) : callback(std::move(other.callback)), args(std::move(other.args))
        {
            id = other.id;
            type = other.type;
            timeout = other.timeout;
            repeats = other.repeats;
            startTs = other.startTs;
            triggered = other.triggered;
            other.id = 0;
            other.timeout = 0;
            other.repeats = 0;
            other.startTs = 0LL;
            other.triggered = false;
        }

        TimerEntryInfo &operator=(TimerEntryInfo &&other)
        {
            callback = std::move(other.callback);
            args = std::move(other.args);
            id = other.id;
            type = other.type;
            timeout = other.timeout;
            repeats = other.repeats;
            startTs = other.startTs;
            triggered = other.triggered;
            other.id = 0;
            other.timeout = 0;
            other.repeats = 0;
            other.startTs = 0LL;
            other.triggered = false;
        }

        TimerEntryInfo &setArgs(const WrappedArgs &_args)
        {
            this->args = _args;
            return *this;
        }

        TimerEntryInfo &setArgs(WrappedArgs &&_args)
        {
            this->args = std::move(_args);
            return *this;
        }

        bool call(void)
        {
            // should be universal, depends only on the callback type and how it's overridden
            if (!callback)
                return false;
            // depending on the implementation, input arguments might get ignored
            // invoke the callback, pass wrapped arguments
            auto status = (*callback)(args);
            repeats = repeats > 0 ? repeats - 1 : repeats; // remove only if more than zero
            triggered = true;
            startTs = timesys::ticks();
            return status;
        }

        inline constexpr bool isInactive(void) const noexcept { return (triggered && type == TIMEOUT) || repeats == 0; }

        inline constexpr bool isInterval(void) const noexcept { return type == INTERVAL; }

        inline constexpr bool isTimeout(void) const noexcept { return type == TIMEOUT; }

        inline Type getType(void) const { return type; }

        inline uint32_t getId(void) const { return id; }

        static uint32_t autoid() { return ++s_autoid; }
    }; //# struct TimerEntryInfo
    //#-----------------------------------------------------------------------------------

    struct TimerHelper
    {
        TimerHelper() = delete;
        TimerHelper(const TimerHelper &other) = delete;

        template <TimerEntryInfo::Type TimerType, typename ReturnType, typename... Args>
        static std::enable_if<TimerType == TimerEntryInfo::INTERVAL, TimerEntryInfo>
        function(int interval, int repeats, ReturnType (*function)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            if (repeats == 0 || repeats < -1)
                repeats = -1;
            return TimerEntryInfo(TimerEntryInfo::autoid(), repeats, timeout, util::FunctionCallback::create<ReturnType, Args...>(function, argNames)); // move
        }

        template <TimerEntryInfo::Type TimerType, typename ReturnType, typename... Args>
        static std::enable_if<TimerType == TimerEntryInfo::TIMEOUT, TimerEntryInfo>
        function(int timeout, ReturnType (*function)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            return TimerEntryInfo(TimerEntryInfo::autoid(), 1, timeout, util::FunctionCallback::create<ReturnType, Args...>(function, argNames)); // move
        }

        template <TimerEntryInfo::Type TimerType, typename UserClass, typename ReturnType, typename... Args>
        static std::enable_if<TimerType == TimerEntryInfo::INTERVAL, TimerEntryInfo>
        method(int interval, int repeats, UserClass *pObject, ReturnType (UserClass::*methodMember)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            if (repeats == 0 || repeats < -1)
                repeats = -1;
            return TimerEntryInfo(TimerEntryInfo::autoid(), repeats, interval, util::MethodCallback::create<UserClass, ReturnType, Args...>(pObject, methodMember, argNames)); // move
        }

        template <TimerEntryInfo::Type TimerType, typename UserClass, typename ReturnType, typename... Args>
        static std::enable_if<TimerType == TimerEntryInfo::TIMEOUT, TimerEntryInfo>
        method(int timeout, UserClass *pObject, ReturnType (UserClass::*methodMember)(Args...), const std::initializer_list<std::string> &argNames = {})
        {
            return TimerEntryInfo(TimerEntryInfo::autoid(), 1, timeout, util::MethodCallback::create<UserClass, ReturnType, Args...>(pObject, methodMember, argNames)); // move
        }

    }; //# struct TimerHelper
    //#-----------------------------------------------------------------------------------
} //> namespace event

#endif //> FG_INC_TIMER_ENTRY_INFO