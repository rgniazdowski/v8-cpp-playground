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
        util::WrappedValue::Args *args;

        ThrownEvent(Type _eventCode = event::Type::Invalid,
                    util::WrappedValue::Args *_args = nullptr) : eventCode(_eventCode),
                                                                 args(_args) {}

        ThrownEvent(const ThrownEvent &other) = delete;
        ThrownEvent &operator=(const ThrownEvent &other) = delete;

        ThrownEvent(ThrownEvent &&other) : eventCode(other.eventCode),
                                           args(other.args)
        {
            other.eventCode = event::Type::Invalid;
            other.args = nullptr;
        }

        ThrownEvent &operator=(ThrownEvent &&other)
        {
            eventCode = other.eventCode;
            args = other.args;
            other.eventCode = event::Type::Invalid;
            other.args = nullptr;
        }

        ~ThrownEvent()
        {
            eventCode = event::Type::Invalid;
            args = nullptr;
        }
    }; //# struct ThrownEvent

} //> namespace event

#endif //> FG_INC_THROWN_EVENT