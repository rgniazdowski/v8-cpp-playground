#include <event/EventManager.hpp>
#include <util/Timesys.hpp>
#include <util/Util.hpp>

event::EventManager::EventManager() : base_type(),
                                      m_eventBinds(),
                                      m_eventsQueue(),
                                      m_timerEntries(),
                                      m_cleanupIntervalId(),
                                      m_markedTimeouts(),
                                      m_eventStructs(),
                                      m_eventStructsFreeSlots()
{
    m_thread.setThreadName("EventManager");
}
//>---------------------------------------------------------------------------------------

event::EventManager::~EventManager()
{
    self_type::destroy();
}
//>---------------------------------------------------------------------------------------

void event::EventManager::pushToFreeSlot(EventBase *ptr)
{
    if (!ptr)
        return;
    m_eventStructsFreeSlots.push_back(reinterpret_cast<event::EventCombined *>(ptr));
}
//>---------------------------------------------------------------------------------------

void event::EventManager::resetArguments(WrappedArgs &args)
{
    for (auto &arg : args)
    {
        if (arg->isExternal() && arg->getExternalPointer<void>() != nullptr)
        {
            auto pStruct = reinterpret_cast<event::EventCombined *>(arg->getExternalPointer<void>());
            if (util::find(m_eventStructs, pStruct) >= 0)
                pushToFreeSlot((EventBase *)pStruct);
        }
    }
    util::reset_arguments(args);
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::destroy(void)
{
    /* mutex events queue */ {
        const std::lock_guard<std::mutex> lock(m_mutexEventsQueue);
        while (!m_eventsQueue.empty())
        {
            resetArguments(m_eventsQueue.front().args);
            m_eventsQueue.pop();
        }
        m_eventsQueue.clear();
    }
    std::vector<event::Type> boundEvents;
    /* mutex event binds */ {
        const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
        m_eventStructsFreeSlots.clear();
        while (m_eventStructs.size())
        {
            EventCombined *ptr = m_eventStructs.back();
            delete ptr;
            m_eventStructs.pop_back();
        } //# for each event structure

        for (auto &it : m_eventBinds)
            boundEvents.push_back(it.first);
    }
    for (auto eventType : boundEvents)
        this->deleteCallbacks(eventType);
    m_eventBinds.clear();
    m_eventStructs.clear();
    /* mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        for (auto &timer : m_timerEntries)
        {
            resetArguments(timer.args); // cleanup arguments (just in case)
            timer.deactivate();         // mark for removal
        }
    }
    removeInactiveTimers();
    /* mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        m_timerEntries.clear();
    }
    m_init.store(false); // mark as deinitialized
    m_cleanupIntervalId = 0;
    return true;
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::initialize(void)
{
    m_eventStructs.reserve(INITIAL_PTR_VEC_SIZE);
    m_init.store(true);
    m_cleanupIntervalId = addInterval<EventManager, size_t>(60 * 1000, this, &EventManager::removeInactiveTimers);
    return true;
}
//>---------------------------------------------------------------------------------------

event::EventBase *event::EventManager::requestEventStruct(Type eventType)
{
    EventCombined *pEventStruct = nullptr;
    if (m_eventStructsFreeSlots.empty())
    {
        pEventStruct = new event::EventCombined(eventType);
        m_eventStructs.push_back(pEventStruct);
    }
    else
    {
        pEventStruct = m_eventStructsFreeSlots.back();
        pEventStruct->setup(eventType);
        m_eventStructsFreeSlots.pop_back();
    }
    return reinterpret_cast<EventBase *>(pEventStruct);
}
//>---------------------------------------------------------------------------------------

event::ThrownEvent &event::EventManager::throwEvent(Type eventCode, WrappedArgs &args)
{
    const std::lock_guard<std::mutex> lock(m_mutexEventsQueue);
    m_eventsQueue.emplace(ThrownEvent(eventCode, args));
    return m_eventsQueue.back();
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::isRegistered(Type eventCode, util::Callback *pCallback)
{
    if (eventCode == event::Type::Invalid || !pCallback)
        return false;
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    auto found = m_eventBinds.find(eventCode);
    if (found == m_eventBinds.end())
        return false;
    return util::find(found->second, pCallback) >= 0;
} //> isRegistered(...)
//>---------------------------------------------------------------------------------------

event::Type event::EventManager::isRegistered(util::Callback *pCallback)
{
    if (!pCallback)
        return event::Type::Invalid;
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    Type foundEvent = event::Type::Invalid;
    for (auto &it : m_eventBinds)
    {
        if (util::find(it.second, pCallback) >= 0)
        {
            foundEvent = it.first;
            break;
        }
    } //# for each bound event type
    return foundEvent;
} //> isRegistered(...)
//>---------------------------------------------------------------------------------------

unsigned int event::EventManager::executeEvent(Type eventCode)
{
    CallbacksVec callbacks;
    unsigned int count = 0;
    {
        const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
        auto found = m_eventBinds.find(eventCode);
        if (found == m_eventBinds.end())
            return 0;
        callbacks = (*found).second; // copy vec
    }
    for (auto callback : callbacks)
    {
        if (!callback)
            continue;
        (*callback)();
        count++;
    } //> for each callback
    return count;
} //> executeEvent(...)
//>---------------------------------------------------------------------------------------

unsigned int event::EventManager::executeEvent(ThrownEvent &thrownEvent)
{
    CallbacksVec callbacks;
    unsigned int count = 0;
    {
        const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
        auto found = m_eventBinds.find(thrownEvent.eventCode);
        if (found == m_eventBinds.end())
        {
            resetArguments(thrownEvent.args);
            return 0;
        }
        callbacks = (*found).second;
    }
    for (auto callback : callbacks)
    {
        if (!callback)
            continue;
        if (thrownEvent.args.size())
            (*callback)(thrownEvent.args);
        else
            (*callback)();
        count++;
    } //> for each callback
    //
    // Thrown event arguments need to be reset/removed after all events are processed.
    // If there is an event structure on the argument's list, it will be retrieved and
    // added back to the free list;
    resetArguments(thrownEvent.args);
    return count;
} //> executeEvent(...)
//>---------------------------------------------------------------------------------------

unsigned int event::EventManager::executeEvent(Type eventCode, WrappedArgs &args)
{
    CallbacksVec callbacks;
    unsigned int count = 0;
    {
        const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
        auto found = m_eventBinds.find(eventCode);
        if (found == m_eventBinds.end())
        {
            resetArguments(args);
            return 0;
        }
        callbacks = (*found).second; // create a copy for calling functions
    }
    for (auto callback : callbacks)
    {
        if (!callback)
            continue;
        if (args.size())
            (*callback)(args);
        else
            (*callback)();
        count++;
    } //> for each callback
    resetArguments(args);
    return count;
} //> executeEvent(...)
//>---------------------------------------------------------------------------------------

util::Callback *event::EventManager::addCallback(Type eventCode, util::Callback *pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return nullptr;
    // Duplicate callbacks are not allowed for the same event (avoid double trigger)
    if (isRegistered(eventCode, pCallback)) //! Lock - event binds
        return nullptr;
    {
        const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
        m_eventBinds[eventCode].push_back(pCallback);
    }
    return pCallback;
} //> addCallback(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeCallback(Type eventCode, util::Callback *pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return false;
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return false;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    for (; cit != end; cit++)
    {
        if (*cit == pCallback)
        {
            m_eventBinds[eventCode].erase(cit);
            return true;
            break;
        }
    } //> for each callback
    return true;
} //> removeCallback(...)
//>---------------------------------------------------------------------------------------

event::CallbacksVec event::EventManager::removeCallbacks(Type eventCode)
{
    if ((int)eventCode < 0)
        return event::CallbacksVec();
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return event::CallbacksVec();
    event::CallbacksVec output;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    for (; cit != end; cit++)
    {
        output.push_back(*cit);
        *cit = nullptr;
    }
    callbacksVec.clear();
    return output;
} //> removeCallbacks(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::deleteCallback(Type eventCode, util::Callback *&pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return false;
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return false;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    for (; cit != end; cit++)
    {
        if (*cit == pCallback)
        {
            m_eventBinds[eventCode].erase(cit);
            delete pCallback;
            pCallback = nullptr;
            return true;
            break;
        }
    } // for each callback
    return true;
} //> deleteCallback(...)
//>---------------------------------------------------------------------------------------

size_t event::EventManager::deleteCallbacks(Type eventCode)
{
    if ((int)eventCode < 0)
        return 0;
    const std::lock_guard<std::mutex> lock(m_mutexEventBinds);
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return 0;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    size_t cnt = 0;
    for (; cit != end; cit++)
    {
        auto *pCallback = *cit;
        if (pCallback)
            delete pCallback;
        *cit = nullptr;
        cnt++;
    } //> for each callback
    callbacksVec.clear();
    return cnt;
} //> deleteCallbacks(...)
//>---------------------------------------------------------------------------------------

uint32_t event::EventManager::addTimeout(util::Callback *pCallback, const int timeout, WrappedArgs &args)
{
    if (!pCallback)
        return 0;
    const std::lock_guard<std::mutex> lock(m_mutexTimers);
    TimerEntryInfo timer(TimerEntryInfo::autoid(), 1, timeout, pCallback);
    if (args.size() > 0)
        timer.setArgs(std::move(args));
    m_timerEntries.emplace(std::move(timer));
    return m_timerEntries.back().getId();
} //> addTimeout(...)
//>---------------------------------------------------------------------------------------

event::TimerEntryInfo *event::EventManager::getTimer(const uint32_t id)
{
    const std::lock_guard<std::mutex> lock(m_mutexTimers);
    for (auto &it : m_timerEntries)
        if (it.getId() == id)
            return &it;
    return nullptr;
} //> getTimer(...)
//>---------------------------------------------------------------------------------------

event::TimerEntryInfo const *event::EventManager::getTimer(const uint32_t id) const
{
    const std::lock_guard<std::mutex> lock(m_mutexTimers);
    for (const auto &it : m_timerEntries)
        if (it.getId() == id)
            return &it;
    return nullptr;
} //> getTimer(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::hasTimer(const uint32_t id) const
{
    const std::lock_guard<std::mutex> lock(m_mutexTimers);
    for (const auto &it : m_timerEntries)
    {
        if (it.getId() == id)
            return true;
    }
    return false;
} //> hasTimer(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeTimer(const uint32_t id)
{
    if (!hasTimer(id)) //! Lock - timers
        return false;
    TimerEntries output;
    // need to go over every entry and emplace it back to the new queue if it's different
    {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        m_markedTimeouts.push_back(id);
        for (auto &it : m_timerEntries)
        {
            if (it.getId() != id)
                output.emplace(std::move(it));
        }
        std::swap(output, m_timerEntries);
    }
    return true;
} //> removeTimer(...)
//>---------------------------------------------------------------------------------------

size_t event::EventManager::removeTimers(const std::vector<uint32_t> &ids)
{
    if (!ids.size())
        return 0;
    size_t cnt = 0;
    for (auto &id : ids)
    {
        if (hasTimer(id)) //! Lock - timers
            cnt++;
    }
    if (!cnt)
        return 0;
    TimerEntries output;
    // need to go over every entry and emplace it back to the new queue if it's different
    /* lock mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        for (auto &it : m_timerEntries)
        {
            auto id = it.getId();
            if (util::find(ids, id) < 0)
                output.emplace(std::move(it));
        }
        std::swap(output, m_timerEntries);
    }
    return cnt;
} //> removeTimers(...)
//>---------------------------------------------------------------------------------------

size_t event::EventManager::removeInactiveTimers(void)
{
    std::vector<uint32_t> ids;
    {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        for (const auto &it : m_timerEntries)
        {
            if (it.isInactive())
                ids.push_back(it.getId());
        }
    }
    return removeTimers(ids);
} //> removeInactiveTimers(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeTimer(const util::Callback *pCallback)
{
    bool found = false;
    uint32_t id = 0;
    /* lock mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        for (const auto &it : m_timerEntries)
        {
            if (it.checkCallback(pCallback))
            {
                id = it.getId();
                found = true;
                break;
            }
        }
    }
    if (found)
        removeTimer(id);
    return found;
} //> removeTimer(...)
//>---------------------------------------------------------------------------------------

uint32_t event::EventManager::addInterval(const int interval, util::Callback *pCallback,
                                          const int repeats, WrappedArgs &args)
{
    if (!pCallback)
        return 0;
    const std::lock_guard<std::mutex> lock(m_mutexTimers);
    TimerEntryInfo timer(TimerEntryInfo::autoid(), repeats, interval, pCallback);
    if (args.size() > 0)
        timer.setArgs(std::move(args));
    m_timerEntries.emplace(std::move(timer));
    return m_timerEntries.back().getId();
} //> addInteval(...)
//>---------------------------------------------------------------------------------------

void event::EventManager::processTimers(void)
{
    //#-----------------------------------------------------------------------------------
    //# Phase 1: Intervals & timeouts - universal
    const auto timeStamp = timesys::ticks();
    TimerEntries timerEntries;
    /* lock mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        timerEntries = std::move(m_timerEntries); // move all the contents for pending timers
    }
    // After timeout is executed it needs to be deleted from the timeouts pool - also the callback pointer must
    // be freed with the argument list as they no longer needed - it will be done automatically in a separate self-owned timer
    for (auto &timer : timerEntries)
    {
        if (timer.isInactive())
            continue; // skip inactive ones
        const auto targetTs = timer.getTargetTs();
        // trigger callback only if target TS is met
        if (timeStamp >= targetTs)
            timer.call();
    } //# for each timer
    std::vector<uint32_t> markedTimeouts;
    /* lock mutex timers */ {
        const std::lock_guard<std::mutex> lock(m_mutexTimers);
        for (auto &it : timerEntries)
            m_timerEntries.emplace(std::move(it));
        timerEntries.clear();
        markedTimeouts = std::move(m_markedTimeouts);
    }
    /* tries to remove again any timers that might've been removed during timer entries loop */
    removeTimers(markedTimeouts);
} //> processTimers(...)
//>---------------------------------------------------------------------------------------

void event::EventManager::processEvents(void)
{
    EventsQueue queue;
    {
        // using mutex and move to another (local) queue in order to avoid recursive lock on queue
        // if new event is thrown while executing given event
        const std::lock_guard<std::mutex> lock(m_mutexEventsQueue);
        queue = std::move(m_eventsQueue); // pending queue is now empty
    }
    //#-----------------------------------------------------------------------------------
    //# Phase 2: execution of thrown events (now including the argument list).
    while (!queue.empty())
    {
        // this will also cleanup the allocated argument list that's associated with thrown event
        executeEvent(queue.front()); //! Lock - event binds
        queue.pop();
    } //> for each event on queue (snapshot)
} //> processEvents(...)
//>---------------------------------------------------------------------------------------

void event::EventManager::processEventsAndTimers(void)
{
    processTimers(); //! Lock - timers
    processEvents(); //! Lock - events queue - event binds
} //> executeEvents(...)
//>---------------------------------------------------------------------------------------
