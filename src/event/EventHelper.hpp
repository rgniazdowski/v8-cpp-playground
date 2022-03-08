#pragma once
#ifndef FG_INC_EVENT_HELPER
#define FG_INC_EVENT_HELPER

#include <util/Callbacks.hpp>

#include <event/EventDefinitions.hpp>
#include <event/ThrownEvent.hpp>
#include <event/TimerEntryInfo.hpp>

#include <map>

#include <Queue.hpp>

namespace event
{
    using CallbacksVec = std::vector<::util::Callback *>;
    using CallbacksBindingMap = std::map<Type, CallbacksVec>;

    using KeyBindingMap = std::map<KeyCode, CallbacksVec>;
    using EventsQueue = Queue<event::ThrownEvent>;

    using TimerEntries = Queue<event::TimerEntryInfo>;
    using EventsPtrVec = std::vector<EventCombined *>;

} //> namespace event

#endif //> FG_INC_EVENT_HELPER
