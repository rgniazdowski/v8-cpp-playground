#pragma once
#ifndef FG_INC_THROWN_EVENT
#define FG_INC_THROWN_EVENT

#include <event/EventDefinitions.hpp>
#include <util/Bindings.hpp>

namespace event
{
    /**
     * @brief Information about thrown event, so this includes event code, pointer to vector
     * holding input arguments (wrapped values / universal) - owned by event manager.
     */
    struct ThrownEvent
    {
        Type eventCode;
        util::WrappedValue::Args args;

        ThrownEvent() : eventCode(Type::Invalid), args() {}

        ThrownEvent(Type _eventCode, util::WrappedValue::Args &_args) : eventCode(_eventCode),
                                                                        args(std::move(_args)) {}

        ThrownEvent(const ThrownEvent &other) = delete;
        ThrownEvent &operator=(const ThrownEvent &other) = delete;

        ThrownEvent(ThrownEvent &&other) noexcept : eventCode(std::exchange(other.eventCode, Type::Invalid)),
                                                    args(std::move(other.args)) {}

        ThrownEvent &operator=(ThrownEvent &&other) noexcept
        {
            eventCode = other.eventCode;
            args = std::move(other.args);
            other.eventCode = Type::Invalid;
        }

        ~ThrownEvent()
        {
            eventCode = Type::Invalid;
            util::reset_arguments(args);
        }
    }; //# struct ThrownEvent

} //> namespace event

#endif //> FG_INC_THROWN_EVENT