#include <event/EventManager.hpp>
#include <util/timesys.hpp>
#include <util/Util.hpp>

event::EventManager::EventManager() : m_eventBinds(),
                                      m_eventsQueue(),
                                      m_timerEntries(),
                                      m_eventStructs(),
                                      m_eventStructsFreeSlots(),
                                      m_argsLists(),
                                      m_argsListsFreeSlots()
{
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

void event::EventManager::pushToFreeSlot(WrappedArgs *pArgs)
{
    if (!pArgs)
        return;
    auto &args = *pArgs;
    for (auto &arg : args)
    {
        if (arg->isExternal() && arg->getExternalPointer() != nullptr)
        {
            auto pStruct = reinterpret_cast<event::EventCombined *>(arg->getExternalPointer());
            if (util::find(m_eventStructs, pStruct) >= 0)
                pushToFreeSlot((EventBase *)pStruct);
        }
    }
    util::reset_arguments(args);
    m_argsListsFreeSlots.push_back(pArgs);
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::destroy(void)
{
    for (auto &it : m_eventBinds)
    {
        auto &callbacks = it.second;
        for (auto &callback : callbacks)
        {
            if (callback)
                delete callback;
            callback = nullptr;
        }
        callbacks.clear();
    }

    while (!m_eventsQueue.empty())
    {
        if (m_eventsQueue.front().args != nullptr)
        {
            // delete m_eventsQueue.front().argList;
            pushToFreeSlot(m_eventsQueue.front().args);
            m_eventsQueue.front().args = nullptr;
        }
        m_eventsQueue.pop();
    }

    for (auto &timer : m_timerEntries)
    {
        // if (timer.args)
        //     pushToFreeSlot(timeout.argList);
        // if (timer.)
        //     delete timeout.callback;
        // timer.args = nullptr;
        // timer.callback = nullptr;
    }
    m_eventStructsFreeSlots.clear();
    while (m_eventStructs.size())
    {
        EventCombined *ptr = m_eventStructs.back();
        delete ptr;
        m_eventStructs.pop_back();
    } //# for each event structure
    for (auto &pItem : m_argsLists)
        pushToFreeSlot(pItem);
    m_argsListsFreeSlots.clear();
    while (m_argsLists.size())
    {
        auto pArgs = m_argsLists.back();
        if (pArgs)
            delete pArgs;
        m_argsLists.pop_back();
    } //# for each argument list
    m_eventBinds.clear();
    m_timerEntries.clear();
    m_init = false;
    return true;
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::initialize(void)
{
    m_eventStructs.reserve(INITIAL_PTR_VEC_SIZE);
    m_argsLists.reserve(INITIAL_PTR_VEC_SIZE);
    m_init = true;
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
        pEventStruct->timeStamp = timesys::ticks();
    }
    else
    {
        pEventStruct = m_eventStructsFreeSlots.back();
        memset(pEventStruct, 0, sizeof(event::EventCombined));
        pEventStruct->eventType = eventType;
        pEventStruct->timeStamp = timesys::ticks();
        m_eventStructsFreeSlots.pop_back();
    }
    return reinterpret_cast<EventBase *>(pEventStruct);
}
//>---------------------------------------------------------------------------------------

event::WrappedArgs *event::EventManager::requestWrappedArgs(void)
{
    WrappedArgs *pArgs = nullptr;
    if (m_argsListsFreeSlots.empty())
    {
        pArgs = new WrappedArgs();
        m_argsLists.push_back(pArgs);
    }
    else
    {
        pArgs = m_argsListsFreeSlots.back();
        util::reset_arguments(*pArgs); //! FIXME
        m_argsListsFreeSlots.pop_back();
    }
    return pArgs;
}
//>---------------------------------------------------------------------------------------

event::ThrownEvent &event::EventManager::throwEvent(Type eventCode, WrappedArgs *args)
{
    m_eventsQueue.emplace(ThrownEvent(eventCode, args));
    return m_eventsQueue.back();
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::isRegistered(Type eventCode, util::Callback *pCallback)
{
    if (eventCode == event::Type::Invalid || !pCallback)
        return false;
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
    unsigned int count = 0;
    auto found = m_eventBinds.find(eventCode);
    if (found == m_eventBinds.end())
        return 0;
    const auto &callbacks = (*found).second;
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

unsigned int event::EventManager::executeEvent(const ThrownEvent &thrownEvent)
{
    unsigned int count = 0;
    auto found = m_eventBinds.find(thrownEvent.eventCode);
    if (found == m_eventBinds.end())
        return 0;
    auto &callbacks = (*found).second;
    for (auto callback : callbacks)
    {
        if (!callback)
            continue;
        if (thrownEvent.args)
            (*callback)(*thrownEvent.args);
        else
            (*callback)();
        count++;
    } //> for each callback
    if (thrownEvent.args)
        pushToFreeSlot(thrownEvent.args);
    return count;
} //> executeEvent(...)
//>---------------------------------------------------------------------------------------

unsigned int event::EventManager::executeEvent(Type eventCode, WrappedArgs *args)
{
    unsigned int count = 0;
    auto found = m_eventBinds.find(eventCode);
    if (found == m_eventBinds.end())
        return 0;
    auto &callbacks = (*found).second;
    for (auto callback : callbacks)
    {
        if (!callback)
            continue;
        if (args)
            (*callback)(*args);
        else
            (*callback)();
        count++;
    } // for each callback
    if (args)
        pushToFreeSlot(args);
    return count;
} //> executeEvent(...)
//>---------------------------------------------------------------------------------------

util::Callback *event::EventManager::addCallback(Type eventCode, util::Callback *pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return nullptr;
    // Duplicate callbacks are not allowed for the same event (avoid double trigger)
    if (isRegistered(eventCode, pCallback))
        return nullptr;
    m_eventBinds[eventCode].push_back(pCallback);
    return pCallback;
} //> addCallback(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeCallback(Type eventCode, util::Callback *pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return false;
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

bool event::EventManager::removeCallbacks(Type eventCode)
{
    if ((int)eventCode < 0)
        return false;
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return false;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    for (; cit != end; cit++)
    {
        *cit = nullptr;
    }
    callbacksVec.clear();
    return true;
} //> removeCallbacks(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::deleteCallback(Type eventCode, util::Callback *&pCallback)
{
    if (!pCallback || (int)eventCode < 0)
        return false;
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

bool event::EventManager::deleteCallbacks(Type eventCode)
{
    if ((int)eventCode < 0)
        return false;
    auto &callbacksVec = m_eventBinds[eventCode];
    if (callbacksVec.empty())
        return false;
    auto cit = callbacksVec.begin(), end = callbacksVec.end();
    for (; cit != end; cit++)
    {
        util::Callback *pCallback = *cit;
        delete pCallback;
        *cit = nullptr;
        pCallback = nullptr;
    } //> for each callback
    callbacksVec.clear();
    return true;
} //> deleteCallbacks(...)
//>---------------------------------------------------------------------------------------

event::TimerEntryInfo *event::EventManager::addTimeout(util::Callback *pCallback,
                                                       const int timeout, WrappedArgs &args)
{
    if (!pCallback)
        return nullptr;
    TimerEntryInfo timer(TimerEntryInfo::autoid(), 1, timeout, pCallback);
    timer.setArgs(std::move(args));
    m_timerEntries.emplace(std::move(timer));
    return &m_timerEntries.back();
} // addTimeoutCallback(...)
//>---------------------------------------------------------------------------------------

bool event::EventManager::hasTimer(const uint32_t id)
{
    for (const auto &it : m_timerEntries)
    {
        if (it.getId() == id)
            return true;
    }
    return false;
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeTimer(const uint32_t id)
{
    if (!hasTimer(id))
        return false;
    TimerEntries output;
    // need to go over every entry and emplace it back to the new queue if it's different
    for (auto &it : m_timerEntries)
    {
        if (it.getId() != id)
            output.emplace(std::move(it));
    }
    std::swap(output, m_timerEntries);
    return true;
}
//>---------------------------------------------------------------------------------------

size_t event::EventManager::removeTimers(const std::vector<uint32_t> &ids)
{
    size_t cnt = 0;
    for (auto &id : ids)
    {
        if (hasTimer(id))
            cnt++;
    }
    if (!cnt)
        return 0;
    TimerEntries output;
    // need to go over every entry and emplace it back to the new queue if it's different
    for (auto &it : m_timerEntries)
    {
        auto id = it.getId();
        if (util::find(ids, id) < 0)
            output.emplace(std::move(it));
    }
    std::swap(output, m_timerEntries);
    return cnt;
}
//>---------------------------------------------------------------------------------------

bool event::EventManager::removeTimer(const util::Callback *pCallback)
{
    return false;
}
//>---------------------------------------------------------------------------------------

event::TimerEntryInfo *event::EventManager::addInterval(const int repeats,
                                                        const int interval,
                                                        util::Callback *pCallback,
                                                        WrappedArgs &args)
{
    if (!pCallback)
        return nullptr;
    TimerEntryInfo timer(TimerEntryInfo::autoid(), repeats, interval, pCallback);
    timer.setArgs(std::move(args));
    m_timerEntries.emplace(std::move(timer));
    return &m_timerEntries.back();
} //> addInteval(...)
//>---------------------------------------------------------------------------------------

void event::EventManager::executeEvents(void)
{
    /*
    ////////////////////////////////////////////////////////////////////////////
    // Phase 1: Timeouts
    unsigned long int timeStamp = timesys::ticks();

    // After timeout is executed it needs to be deleted from the timeouts pool - also the callback pointer must
    // be freed with the argument list as they no longer needed
    for (TimeoutCallbacksVecItor it = m_timeoutCallbacks.begin(); it != m_timeoutCallbacks.end(); it++)
    {
        if (timeStamp - it->timeStamp < (unsigned long int)it->timeout)
        {
            continue; // skip!
        }
        if (it->callback)
        {
            (*it->callback)(it->argList);
            delete it->callback;
            it->callback = nullptr;
        }
        if (it->argList)
        {
            // delete it->argList;
            pushToFreeSlot(it->argList);
            it->argList = nullptr;
        }
        if (it->callback == nullptr)
        {
            m_timeoutCallbacks.erase(it);
            it--;
        }
    } // for each timeout callback

    ////////////////////////////////////////////////////////////////////////////
    // Phase 2: Cyclic callbacks
    for (int cIdx = 0; cIdx < (int)m_cyclicCallbacks.size(); cIdx++)
    {
        CyclicCallback &cyclic = m_cyclicCallbacks[cIdx];
        if (timeStamp - cyclic.timeStamp < (unsigned long int)cyclic.interval)
        {
            continue; // skip!
        }
        if (cyclic.callback && (cyclic.repeats || cyclic.repeats == -1))
        {
            (*cyclic.callback)(cyclic.argList);
            cyclic.timeStamp = timeStamp;
            if (cyclic.repeats != 0)
                cyclic.repeats--;
        }
        if (cyclic.callback == nullptr)
            cyclic.repeats = 0;

        if (cyclic.repeats != 0)
        {
            continue;
        }
        if (cyclic.callback)
        {
            delete cyclic.callback;
            cyclic.callback = nullptr;
        }
        if (cyclic.argList)
        {
            // delete cyclic.argList;
            pushToFreeSlot(cyclic.argList);
            cyclic.argList = nullptr;
        }
        int n = (int)m_cyclicCallbacks.size();
        m_cyclicCallbacks[cIdx] = m_cyclicCallbacks[n - 1];
        memset(&m_cyclicCallbacks[n - 1], 0, sizeof(CyclicCallback));
        m_cyclicCallbacks.resize(n - 1);
        cIdx--;
    } // for each cyclic callback

    ////////////////////////////////////////////////////////////////////////////
    // Phase 3: execution of thrown events (now including the argument list).
    // Btw after calling the proper callback,
    // queue entry with argument list must be erased
    while (!m_eventsQueue.empty())
    {
        ThrownEvent &event = m_eventsQueue.front();
        executeEvent(event);
        m_eventsQueue.pop();
    } // for each event on queue
    */
} //> executeEvents(...)
//>---------------------------------------------------------------------------------------
